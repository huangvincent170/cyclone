#include <stdlib.h>
#include "cyclone.hpp"
#include "libcyclone.hpp"
#include "../core/clock.hpp"
#include "../core/cyclone_comm.hpp"
#include "../core/logging.hpp"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <unistd.h>
#include "cyclone_context.hpp"


extern dpdk_context_t *global_dpdk_context;
typedef struct rpc_client_st {
  int me;
  int me_mc;
  int me_queue;  // synch rcv queue
  quorum_switch *router;
  rpc_t *packet_out;
  rpc_t *packet_out_aux;
  msg_t *packet_rep;
  rpc_t *packet_in;
  int server;
  int replicas;
  unsigned long channel_seq;
  dpdk_rx_buffer_t *buf;
  int server_ports;
  unsigned int *terms;

	//async client
	int me_aqueue;
  unsigned long channel_aseq;
	struct rte_ring *to_lstnr;	
	volatile char msg_inflight;
	unsigned long async_max_inflight;
	struct async_listener_t_ *lstnr;

  int quorum_q(int quorum_id, int q)
  {
    return server_ports + q*num_quorums + quorum_id;
  }
  
  int choose_quorum(unsigned long core_mask)
  {
    if(core_mask & (core_mask - 1)) {
      return 0;
    }
    else {
      return core_to_quorum(__builtin_ffsl(core_mask) - 1);
    }
  }

  int common_receive_loop(int blob_sz)
  {
    int resp_sz;
    rte_mbuf *junk[PKT_BURST];
#ifdef WORKAROUND0
    // Clean out junk
    if(me_queue == 1) {
      int junk_cnt = cyclone_rx_burst(0, 0, &junk[0], PKT_BURST);
      for(int i=0;i<junk_cnt;i++) {
	rte_pktmbuf_free(junk[i]);
      }
    }
#endif
    while(true) {
      resp_sz = cyclone_rx_timeout(global_dpdk_context,
				   0,
				   me_queue,
				   buf,
				   (unsigned char *)packet_in,
				   MSG_MAXSIZE,
				   timeout_msec*1000);
      if(resp_sz == -1) {
	break;
      }

      if(packet_in->channel_seq != (channel_seq - 1)) {
	BOOST_LOG_TRIVIAL(warning) << "Channel seq mismatch";
	continue;
      }
      
      break;
    }
    return resp_sz;
  }

  void send_to_server(rpc_t *pkt, int sz, int quorum_id, int flags)
  {
    rte_mbuf *mb = rte_pktmbuf_alloc(global_dpdk_context->mempools[me_queue]);
    if(mb == NULL) {
      BOOST_LOG_TRIVIAL(fatal) << "Out of mbufs for send to server";
    }
    pkt->quorum_term = terms[quorum_id];
    cyclone_prep_mbuf_client2server(global_dpdk_context,
				    queue2port(quorum_q(quorum_id, q_dispatcher), server_ports),
				    router->replica_mc(server),
				    queue_index_at_port(quorum_q(quorum_id, q_dispatcher), server_ports),
				    mb,
				    pkt,
				    sz);

		if(flags & ASYNC_REQUEST){
			if(rte_ring_sp_enqueue(to_lstnr, (void *)m) == -ENOBUFS ){
				BOOST_LOG_TRIVIAL(fatal) << "Failed to enqueue listner queue";	
				exit(-1);
			}
			int e = cyclone_tx(global_dpdk_context, mb, me_aqueue);
			if(e) 
				BOOST_LOG_TRIVIAL(warning) << "Client failed to send async request to server";
		}else{
			int e = cyclone_tx(global_dpdk_context, mb, me_queue);
			if(e) 
				BOOST_LOG_TRIVIAL(warning) << "Client failed to send sync request to server";
    }
  }


  
  int set_server()
  {
    packet_out_aux->code        = RPC_REQ_STABLE;
    packet_out_aux->flags       = 0;
    packet_out_aux->core_mask   = 1; // Always core 0
    packet_out_aux->client_port = me_queue;
    packet_out_aux->channel_seq = channel_seq++;
    packet_out_aux->client_id   = me;
    packet_out_aux->requestor   = me_mc;
    packet_out_aux->payload_sz  = 0;
    send_to_server(packet_out_aux, sizeof(rpc_t), 0); // always quorum 0
    int resp_sz = common_receive_loop(sizeof(rpc_t));
    if(resp_sz != -1) {
      memcpy(terms, packet_in + 1, num_quorums*sizeof(unsigned int));
      for(int i=0;i<num_quorums;i++) {
	terms[i] = terms[i] >> 1;
      }
      return 1;
    }
    return 0;
  }

  void update_server(const char *context)
  {
    BOOST_LOG_TRIVIAL(info) 
      << "CLIENT DETECTED POSSIBLE FAILED LEADER: "
      << server
      << " Reason " 
      << context;
    do {
      server = (server + 1)%replicas;
      BOOST_LOG_TRIVIAL(info) << "Trying " << server;
    } while(!set_server());
    BOOST_LOG_TRIVIAL(info) << "Success";
  }



  int delete_node(unsigned long core_mask, int nodeid)
  {
    int retcode;
    int resp_sz;
    int quorum_id = choose_quorum(core_mask);
    while(true) {
      packet_out->code        = RPC_REQ_NODEDEL;
      packet_out->flags       = 0;
      packet_out->core_mask   = core_mask;
      packet_out->client_port = me_queue;
      packet_out->channel_seq = channel_seq++;
      packet_out->client_id   = me;
      packet_out->requestor   = me_mc;
      packet_out->payload_sz  = sizeof(cfg_change_t);
      cfg_change_t *cfg = (cfg_change_t *)(packet_out + 1);
      cfg->node = nodeid;
      send_to_server(packet_out, sizeof(rpc_t) + sizeof(cfg_change_t), quorum_id);
      resp_sz = common_receive_loop(sizeof(rpc_t) + sizeof(cfg_change_t));
      if(resp_sz == -1) {
	update_server("rx timeout");
	continue;
      }
      if(packet_in->code == RPC_REP_FAIL) {
	continue;
      }
      break;
    }
    return 0;
  }

  int add_node(unsigned long core_mask, int nodeid)
  {
    int retcode;
    int resp_sz;
    int quorum_id = choose_quorum(core_mask);
    while(true) {
      packet_out->code        = RPC_REQ_NODEADD;
      packet_out->flags       = 0;
      packet_out->core_mask     = core_mask;
      packet_out->client_port = me_queue;
      packet_out->channel_seq = channel_seq++;
      packet_out->client_id   = me;
      packet_out->requestor   = me_mc;
      packet_out->payload_sz  = sizeof(cfg_change_t);
      cfg_change_t *cfg = (cfg_change_t *)(packet_out + 1);
      cfg->node      = nodeid;
      send_to_server(packet_out, sizeof(rpc_t) + sizeof(cfg_change_t), quorum_id);
      resp_sz = common_receive_loop(sizeof(rpc_t) + sizeof(cfg_change_t));
      if(resp_sz == -1) {
	update_server("rx timeout");
	continue;
      }
      if(packet_in->code == RPC_REP_FAIL) {
	continue;
      }
      break;
    }
    return 0;
  }

  int make_rpc(void *payload, int sz, void **response, unsigned long core_mask, int flags)
  {
    int retcode;
    int resp_sz;
    int quorum_id = choose_quorum(core_mask);
    while(true) {
      // Make request
      packet_out->code        = RPC_REQ;
      packet_out->flags       = flags;
      packet_out->core_mask   = core_mask;
      packet_out->client_port = me_queue;
      packet_out->channel_seq = channel_seq++;
      packet_out->client_id   = me;
      packet_out->requestor   = me_mc;
      if((core_mask & (core_mask - 1)) != 0) {
	char *user_data = (char *)(packet_out + 1);
	memcpy(user_data, terms, num_quorums*sizeof(unsigned int));
	user_data += num_quorums*sizeof(unsigned int);
	user_data += sizeof(ic_rdv_t);
	memcpy(user_data, payload, sz);
	packet_out->payload_sz  = 
	  num_quorums*sizeof(unsigned int) +
	  sizeof(ic_rdv_t) + 
	  sz;
	unsigned int pkt_sz =  packet_out->payload_sz + sizeof(rpc_t);
	send_to_server(packet_out, 
		       pkt_sz,
		       quorum_id);
	resp_sz = common_receive_loop(pkt_sz);
      }
      else {
	packet_out->payload_sz = sz;
	memcpy(packet_out + 1, payload, sz);
	send_to_server(packet_out, sizeof(rpc_t) + sz, quorum_id);
	resp_sz = common_receive_loop(sizeof(rpc_t) + sz);
      }
      if(resp_sz == -1) {
	update_server("rx timeout, make rpc");
	continue;
      }
      if(packet_in->code == RPC_REP_FAIL) {
	continue;
      }
      break;
    }
    *response = (void *)(packet_in + 1);
    return (int)(resp_sz - sizeof(rpc_t));
  }


  int make_rpc_async(void *payload, int sz, void (*cb)(), unsigned long core_mask, int flags)
  {
		int retcode;
    int resp_sz;
    int quorum_id = choose_quorum(core_mask);

			//admission control
			if(get_inflight() >= async_max_inflight)
				return ASYNC_MAX_INFLIGHT;

      // Make request
      packet_out->code        = RPC_REQ;
      packet_out->flags       = flags;
      packet_out->core_mask   = core_mask;
      packet_out->client_port = me_aqueue;
      packet_out->channel_seq = channel_aseq++;
      packet_out->client_id   = me;
      packet_out->requestor   = me_mc;
      if((core_mask & (core_mask - 1)) != 0) {
				BOOST_LOG_TRIVIAL(info) << "gang operations not supported";	      
			} else {
				packet_out->payload_sz = sz;
				memcpy(packet_out + 1, payload, sz);
				send_to_server(packet_out, sizeof(rpc_t) + sz, quorum_id,flags);
      }
			return ASYNC_SEND_SUCCESS;
  }

} rpc_client_t;

/* rx loop of async client */
typedef struct async_lstener_t_{

void exec(){
	unsigned long mark = rte_get_tsc_cycles();
	double tsc_mhz = (rte_get_tsc_hz()/1000000.0);
	unsigned long PERIODICITY_CYCLES = PERIODICITY*tsc_mhz;
	unsigned long LOOP_TO_CYCLES     = ASYNC__TIMEOUT*tsc_mhz;
	unsigned long elapsed_time;
	unsigned int seq;
	int32_t ret,available;
	*pkt_array[PKT_BURST]
	rte_mbuf *m = NULL;
	while(!terminate){
	available = cyclone_rx_burst(0,
				   me_aqueue,
				   &pkt_array[0],
				   PKT_BURST);
	if(available ){
		for(int i=0;i<available;i++) {
			m = pkt_array[i];
			struct ether_hdr *e = rte_pktmbuf_mtod(m, struct ether_hdr *); 
			struct ipv4_hdr *ip = (struct ipv4_hdr *)(e + 1); 
			if(e->ether_type != rte_cpu_to_be_16(ETHER_TYPE_IPv4)) {
				BOOST_LOG_TRIVIAL(warning) << "Dropping junk. Protocol mismatch";
				rte_pktmbuf_free(m);
				return -1; 
			}   
			else if(ip->src_addr != magic_src_ip) {
				BOOST_LOG_TRIVIAL(warning) << "Dropping junk. non magic ip";
				rte_pktmbuf_free(m);
				return -1; 
			}   
			else if(m->data_len <= sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr)) {
				BOOST_LOG_TRIVIAL(warning) << "Dropping junk = pkt size too small";
				rte_pktmbuf_free(m);
				return -1; 
			}
			int payload_offset = sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr);
			void *payload = rte_pktmbuf_mtod_offset(m, void *, payload_offset);
			int msg_size = m->data_len - payload_offset;
			//admission control based on seq no. incoming seq-number should be >= current msg
			unsigned int mseq = m->seq;	
			if(mseq < seq){
				BOOST_LOG_TRIVIAL(warning) << "seq mismatch, current seq : " 
					<< std::to_string(seq) << " message seq : " 
					<< std::to_string(mseq);
			}
			ret = rte_hash_add(clnt->rcvmsg_tbl,(void *)&mseq, (void*)m);
			if(ret < 0 ){
				BOOST_LOG_TRIVIAL(fatal) << "Unable to add hash entry" 
					<< std::to_string(mseq);
					exit(-1);
			}	
		}
	}
	elapsed_time = rte_get_tsc_cycles() - mark;
	if(elapsed_time >= LOOP_TO_CYCLES) {
		BOOST_LOG_TRIVIAL(warning) << "Client " 
			<< " event loop too long cycles = " << elapsed_time;
	}
	/* attend to next sent message */
	if(curr_m == NULL && (rte_ring_sc_dequeue(to_lstnr,(void **)&cur_m) != 0) ){
		continue; // no sent msgs
	}
  elapsed_time = rte_get_tsc_cycles - cur_m->sent_time;	
	if(elapsed_time >= ASYNC__TIMEOUT){
		BOOST_LOG_TRIVIAL(debug) << "message timeout";
		rte_pktmbuf_free(cur_m);
		//TOO:update server
		curr_m->cb();
	}
	
	// lookup received msgs
	ret = ret_hash_lookup(clnt->rcvmsg_tbl,(const void *)&curr_m);
	if(ret < 0)
		continue;
	else{	
		
		rte_pktmbuf_free(cur_m);
		rte_pkmbuf_free(lookedup_m);
		curr_m->cb();
	}
	current = NULL; // on to next one
	}
}

}async_lstener_t;



int async_listener(void *arg){
	async_lstener_t *lst = (async_lstener_t *) arg;
	rte_cpuset_t set;
	rte_thread_get_affinity(&set);
	BOOST_LOG_TRIVIAL(info) << "App Thread launch, affinity = "
										<< get_cpuset(&set);
	lst->exec();
	return 0;
}

/* launch async rx/tx queues */
void cyclone_client_launch(rpc_client_t *handle ,lcore_function_t * f, void* arg, unsigned slave_id)
{			//first launch rx
			int e = rte_eal_remote_launch(async_listener, handle->lstnr, slave_id+1);
	    if(e != 0) {
			  BOOST_LOG_TRIVIAL(fatal) << "Failed to launch receiver on remote lcore";
				exit(-1);
			}
			e = rte_eal_remote_launch(f, arg, slave_id);
	    if(e != 0) {
			  BOOST_LOG_TRIVIAL(fatal) << "Failed to launch driver on remote lcore";
				exit(-1);
			}
}


void add_inflight()
{
	__sync_fetch_and_add(&msg_inflight, 1);
}

void sub_inflight()
{
	__sync_fetch_and_sub(&msg_inflight, 1);
}

char get_inflight()
{
	return msg_inflight;
}





void* cyclone_client_init(int client_id,
			  int client_mc,
			  int client_queue,
			  const char *config_cluster,
			  int server_ports,
			  const char *config_quorum,unsigned int flags)
{
  rpc_client_t * client = new rpc_client_t();
  boost::property_tree::ptree pt_cluster;
  boost::property_tree::ptree pt_quorum;
  boost::property_tree::read_ini(config_cluster, pt_cluster);
  boost::property_tree::read_ini(config_quorum, pt_quorum);
  std::stringstream key;
  std::stringstream addr;
  client->me     = client_id;
  client->router = new quorum_switch(&pt_cluster, &pt_quorum);
  client->terms  = (unsigned int *)malloc(num_quorums*sizeof(unsigned int));
  client->me_mc = client_mc;
  client->me_queue = client_queue;
  client->buf = (dpdk_rx_buffer_t *)malloc(sizeof(dpdk_rx_buffer_t));
  client->buf->buffered = 0;
  client->buf->consumed = 0;
  client->server_ports = server_ports;
  void *buf = new char[MSG_MAXSIZE];
  client->packet_out = (rpc_t *)buf;
  buf = new char[MSG_MAXSIZE];
  client->packet_out_aux = (rpc_t *)buf;
  buf = new char[MSG_MAXSIZE];
  client->packet_in = (rpc_t *)buf;
  buf = new char[MSG_MAXSIZE];
  client->packet_rep = (msg_t *)buf;
  client->replicas = pt_quorum.get<int>("quorum.replicas");
	if(flags & CLIENT_ASYNC){
		client->lstnr = new async_listener_t();

		//initialize comm rings for listerner thread interation
		char ringname[50];
		snprintf(ringname,50,"TO_LSTNR");
		client->to_lstnr = rte_ring_create(ringname,
							 65536,
							 rte_socket_id(),
							 RING_F_SC_DEQ);
		struct rte_hash_parameters hash_params = {
			.name = NULL,
			.key_len = sizeof(unsigned int),
			.entries = UINT_MAX,
			.hash_func = rte_hash_crc	// default		
		};
		client->rcvmsg__tbl= rte_hash_create(&hash_params);
	}		
  client->channel_seq = client_queue*client_mc*rtc_clock::current_time();
  for(int i=0;i<num_quorums;i++) {
    client->server = 0;
    client->update_server("Initialization");
  }
  return (void *)client;
}


int make_rpc_async(void *handle, 
		void *payload, 
		int sz, void (*cb)(), 
		unsigned core_mask, 
		int flags)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  if(sz > DISP_MAX_MSGSIZE) {
    BOOST_LOG_TRIVIAL(fatal) << "rpc call params too large "
			     << " param size =  " << sz
			     << " DISP_MAX_MSGSIZE = " << DISP_MAX_MSGSIZE;
    exit(-1);
  }
  return client->make_rpc_async(payload, sz, cb, core_mask, flags);
}

int make_rpc(void *handle,
	     void *payload,
	     int sz,
	     void **response,
	     unsigned long core_mask,
	     int flags)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  if(sz > DISP_MAX_MSGSIZE) {
    BOOST_LOG_TRIVIAL(fatal) << "rpc call params too large "
			     << " param size =  " << sz
			     << " DISP_MAX_MSGSIZE = " << DISP_MAX_MSGSIZE;
    exit(-1);
  }
  return client->make_rpc(payload, sz, response, core_mask, flags);
}

int delete_node(void *handle, unsigned long core_mask, int node)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  return client->delete_node(core_mask, node);
}

int add_node(void *handle, unsigned long core_mask, int node)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  return client->add_node(core_mask, node);
}
