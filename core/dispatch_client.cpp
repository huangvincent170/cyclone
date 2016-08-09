#include <stdlib.h>
#include "cyclone.hpp"
#include "libcyclone.hpp"
#include "../core/clock.hpp"
#include "../core/cyclone_comm.hpp"
#include "../core/logging.hpp"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <unistd.h>
#include "tuning.hpp"
#include "cyclone_context.hpp"


extern dpdk_context_t *global_dpdk_context;
typedef struct rpc_client_st {
  int me;
  int me_mc;
  int me_queue;
  quorum_switch *router;
  rpc_t *packet_out;
  msg_t *packet_rep;
  rpc_t *packet_in;
  int server;
  int replicas;
  unsigned long channel_seq;
  dpdk_rx_buffer_t *buf;
  
  void update_server(const char *context)
  {
    BOOST_LOG_TRIVIAL(info) 
      << "CLIENT DETECTED POSSIBLE FAILED LEADER: "
      << server
      << " Reason " 
      << context;
    server = (server + 1)%replicas;
    BOOST_LOG_TRIVIAL(info) << "CLIENT SET NEW LEADER " << server;
  }

  void set_server()
  {
    BOOST_LOG_TRIVIAL(info) << "CLIENT SETTING LEADER " << server;
  }


  int common_receive_loop(int blob_sz)
  {
    int retcode;
    int resp_sz;
    bool sent_assist_msg   = false;
    bool sent_assist_reply = false;
    unsigned long response_map = 0;
    while(true) {
      resp_sz = cyclone_rx_timeout(global_dpdk_context,
				   me_queue,
				   buf,
				   (unsigned char *)packet_in,
				   MSG_MAXSIZE,
				   timeout_msec*1000);
      if(resp_sz == -1) {
	break;
      }

      if(packet_in->channel_seq != (channel_seq - 1)) {
	continue;
      }
      
      break;
    }
    return resp_sz;
  }

  void send_to_server(int sz)
  {
    rte_mbuf *mb = rte_pktmbuf_alloc(global_dpdk_context->mempools[me_queue]);
    if(mb == NULL) {
      BOOST_LOG_TRIVIAL(fatal) << "Out of mbufs for send requestvote";
    }
    cyclone_prep_mbuf(global_dpdk_context,
		      router->replica_mc(server),
		      q_dispatcher,
		      mb,
		      packet_out,
		      sz);
    cyclone_tx(global_dpdk_context, mb, me_queue);
  }

  int get_last_txid()
  {
    int retcode;
    int resp_sz;
    while(true) {
      packet_out->code        = RPC_REQ_LAST_TXID;
      packet_out->flags       = 0;
      packet_out->client_id   = me;
      packet_out->client_port = me_queue;
      packet_out->client_txid = (int)packet_out->timestamp;
      packet_out->channel_seq = channel_seq++;
      packet_out->requestor   = me_mc;
      packet_out->payload_sz  = 0;
      send_to_server(sizeof(rpc_t));
      while(true) {
	resp_sz = cyclone_rx_timeout(global_dpdk_context,
				     me_queue,
				     buf,
				     (unsigned char *)packet_in,
				     MSG_MAXSIZE,
				     timeout_msec*1000);
	if(resp_sz == -1) {
	  break;
	}

	if(packet_in->channel_seq != (channel_seq - 1)) {
	  continue;
	}

	break;
      }
      if(resp_sz == -1) {
	update_server("rx timeout, get txid");
	continue;
      }
      if(packet_in->code == RPC_REP_INVSRV) {
	update_server("Server not leader");
	continue;
      }
      break;
    }
    return packet_in->last_client_txid;
  }

  int delete_node(int txid, int nodeid)
  {
    int retcode;
    int resp_sz;
    while(true) {
      packet_out->code        = RPC_REQ_NODEDEL;
      packet_out->flags       = 0;
      packet_out->client_id   = me;
      packet_out->client_port = me_queue;
      packet_out->client_txid = txid;
      packet_out->channel_seq = channel_seq++;
      packet_out->requestor   = me_mc;
      packet_out->payload_sz  = sizeof(cfg_change_t);
      cfg_change_t *cfg = (cfg_change_t *)(packet_out + 1);
      cfg->node = nodeid;
      send_to_server(sizeof(rpc_t) + sizeof(cfg_change_t));
      resp_sz = common_receive_loop(sizeof(rpc_t) + sizeof(cfg_change_t));
      if(resp_sz == -1) {
	update_server("rx timeout");
	continue;
      }
      if(packet_in->code == RPC_REP_INVSRV) {
	update_server("Server not leader");
	continue;
      }
      if(packet_in->code == RPC_REP_UNKNOWN) {
	continue;
      }
      break;
    }
    if(packet_in->code == RPC_REP_OLD) {
      return RPC_EOLD;
    }
    return 0;
  }

  int add_node(int txid, int nodeid)
  {
    int retcode;
    int resp_sz;
    while(true) {
      packet_out->code        = RPC_REQ_NODEADD;
      packet_out->flags       = 0;
      packet_out->client_id   = me;
      packet_out->client_port = me_queue;
      packet_out->client_txid = txid;
      packet_out->channel_seq = channel_seq++;
      packet_out->requestor   = me_mc;
      packet_out->payload_sz  = sizeof(cfg_change_t);
      cfg_change_t *cfg = (cfg_change_t *)(packet_out + 1);
      cfg->node      = nodeid;
      send_to_server(sizeof(rpc_t) + sizeof(cfg_change_t));
      resp_sz = common_receive_loop(sizeof(rpc_t) + sizeof(cfg_change_t));
      if(resp_sz == -1) {
	update_server("rx timeout");
	continue;
      }
      if(packet_in->code == RPC_REP_INVSRV) {
	update_server("Server not leader");
	continue;
      }
      break;
    }
    return packet_in->last_client_txid;
  }

  int retrieve_response(void **response, int txid)
  {
    int retcode;
    int resp_sz;
    packet_out->client_id   = me;
    packet_out->client_port = me_queue;
    packet_out->client_txid = txid;
    packet_out->channel_seq  = channel_seq++;
    packet_out->requestor   = me_mc;
    while(true) {
      packet_out->code        = RPC_REQ_STATUS;
      packet_out->flags       = 0;
      send_to_server(sizeof(rpc_t));
      while(true) {
	resp_sz = cyclone_rx_timeout(global_dpdk_context,
				     me_queue,
				     buf,
				     (unsigned char *)packet_in,
				     MSG_MAXSIZE,
				     timeout_msec*1000);
	if(resp_sz == -1) {
	  break;
	}
	
	if(packet_in->channel_seq != (channel_seq - 1)) {
	  continue;
	}

	break;
      }
      if(resp_sz == -1) {
	update_server("rx timeout, get response");
	continue;
      }
      if(packet_in->code == RPC_REP_INVSRV) {
	update_server("Server not leader");
	continue;
      }
      break;
    }
    if(packet_in->code == RPC_REP_OLD) {
      return RPC_EOLD;
    }
    if(packet_in->code == RPC_REP_UNKNOWN) {
      return RPC_EUNKNOWN;
    }
    *response = (void *)(packet_in + 1);
    return (int)(resp_sz - sizeof(rpc_t));
  }
  
  int make_rpc(void *payload, int sz, void **response, int txid, int flags)
  {
    int retcode;
    int resp_sz;
    while(true) {
      // Make request
      packet_out->code        = RPC_REQ_FN;
      packet_out->flags       = flags;
      packet_out->client_id   = me;
      packet_out->client_port = me_queue;
      packet_out->client_txid = txid;
      packet_out->channel_seq = channel_seq++;
      packet_out->requestor   = me_mc;
      packet_out->payload_sz  = sz;
      memcpy(packet_out + 1, payload, sz);
      send_to_server(sizeof(rpc_t) + sz);
      resp_sz = common_receive_loop(sizeof(rpc_t) + sz);
      if(resp_sz == -1) {
	update_server("rx timeout, make rpc");
	continue;
      }
      if(packet_in->code == RPC_REP_INVSRV) {
	update_server("Server not leader");
	continue;
      }
      if(packet_in->code == RPC_REP_UNKNOWN) {
	continue;
      }
      break;
    }
    if(packet_in->code == RPC_REP_OLD) {
      return RPC_EOLD;
    }
    *response = (void *)(packet_in + 1);
    return (int)(resp_sz - sizeof(rpc_t));
  }

  int make_noop_rpc(int txid, int flags)
  {
    int retcode;
    int resp_sz;
    while(true) {
      // Make request
      packet_out->code        = RPC_REQ_NOOP;
      packet_out->flags       = flags;
      packet_out->client_id   = me;
      packet_out->client_port = me_queue;
      packet_out->client_txid = txid;
      packet_out->channel_seq = channel_seq++;
      packet_out->requestor   = me_mc;
      packet_out->payload_sz  = 0;
      send_to_server(sizeof(rpc_t));
      resp_sz = common_receive_loop(sizeof(rpc_t));
      if(resp_sz == -1) {
	update_server("rx timeout, make rpc");
	continue;
      }
      if(packet_in->code == RPC_REP_INVSRV) {
	update_server("Server not leader");
	continue;
      }
      if(packet_in->code == RPC_REP_UNKNOWN) {
	continue;
      }
      break;
    }
    if(packet_in->code == RPC_REP_OLD) {
      return RPC_EOLD;
    }
    return 0;
  }
  
} rpc_client_t;


void* cyclone_client_init(int client_id,
			  int client_mc,
			  int client_queue,
			  const char *config_cluster,
			  const char *config_quorum)
{
  rpc_client_t * client = new rpc_client_t();
  boost::property_tree::ptree pt_cluster;
  boost::property_tree::ptree pt_quorum;
  boost::property_tree::read_ini(config_cluster, pt_cluster);
  boost::property_tree::read_ini(config_quorum, pt_quorum);
  std::stringstream key;
  std::stringstream addr;
  client->router = new quorum_switch(&pt_cluster, &pt_quorum);
  client->me = client_id;
  client->me_mc = client_mc;
  client->me_queue = client_queue;
  client->buf = (dpdk_rx_buffer_t *)malloc(sizeof(dpdk_rx_buffer_t));
  client->buf->buffered = 0;
  client->buf->consumed = 0;
  void *buf = new char[MSG_MAXSIZE];
  client->packet_out = (rpc_t *)buf;
  buf = new char[MSG_MAXSIZE];
  client->packet_in = (rpc_t *)buf;
  buf = new char[MSG_MAXSIZE];
  client->packet_rep = (msg_t *)buf;
  client->replicas = pt_quorum.get<int>("quorum.replicas");
  client->channel_seq = client_id*client_mc*rtc_clock::current_time();
  client->server = 0;
  client->set_server();
  return (void *)client;
}

int make_rpc(void *handle,
	     void *payload,
	     int sz,
	     void **response,
	     int txid,
	     int flags)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  if(sz > DISP_MAX_MSGSIZE) {
    BOOST_LOG_TRIVIAL(fatal) << "rpc call params too large "
			     << " param size =  " << sz
			     << " DISP_MAX_MSGSIZE = " << DISP_MAX_MSGSIZE;
    exit(-1);
  }
  return client->make_rpc(payload, sz, response, txid, flags);
}

int make_noop_rpc(void *handle,
		  int txid,
		  int flags)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  return client->make_noop_rpc(txid, flags);
}

int get_last_txid(void *handle)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  return client->get_last_txid();
}

int get_response(void *handle, void **response, int txid)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  return client->retrieve_response(response, txid);
}

int delete_node(void *handle, int txid, int node)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  return client->delete_node(txid, node);
}

int add_node(void *handle, int txid, int node)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  return client->add_node(txid, node);
}
