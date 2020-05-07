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
#include "tcp_tunnel.hpp"


const static unsigned int SYNC_REQUEST = 1 << 11;
const static unsigned int ASYNC_REQUEST = 1 << 10;

/* comm struct between async client sender/receiver */
typedef struct async_comm_st{
    int payload_sz;
    unsigned long channel_seq;
    unsigned long timestamp;
    void (*cb)(void *, int, unsigned long);
    void *cb_args;
}async_comm_t;




extern dpdk_context_t *global_dpdk_context;
typedef struct rpc_client_st {
  tunnel_t *client2server_tunnels;
  int me;
  int me_mc;
  int me_queue;
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
    //struct rte_hash *sentmsg_tbl;
    std::map<unsigned long, async_comm_t *> *pendresponse_map; // ordered map 
    volatile int64_t msg_inflight;
    unsigned long max_inflight; 



  tunnel_t* client2server_tunnel(int server, int quorum)
  {
    return &client2server_tunnels[server*num_quorums + quorum];
  }

  int quorum_q(int quorum_id, int q)
  {
    return server_ports + num_queues*quorum_id + q;
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

  int common_receive_loop(int blob_sz, int quorum)
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
      if(!client2server_tunnel(server, quorum)->receive_timeout(timeout_msec*10000)) {
      	BOOST_LOG_TRIVIAL(info) << "msg timeout in common loop";
	resp_sz = -1;
	break;
      }
      rte_mbuf *mb = rte_pktmbuf_alloc(global_dpdk_context->mempools[me_queue]);
      if(mb == NULL) {
	BOOST_LOG_TRIVIAL(fatal) << "no mbufs for client rcv";
	exit(-1);
      }
      client2server_tunnel(server, quorum)->copy_out(mb);
      int payload_offset = sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr);
      void *payload = rte_pktmbuf_mtod_offset(mb, void *, payload_offset);
      resp_sz = mb->data_len - payload_offset;
      rte_memcpy(packet_in, payload, resp_sz);
      rte_pktmbuf_free(mb);
      if(packet_in->channel_seq != (channel_seq - 1)) {
	BOOST_LOG_TRIVIAL(warning) << "Channel seq mismatch";
	continue;
      }
      BOOST_LOG_TRIVIAL(info) << "msg rcv in common loop";
      break;
    }
    return resp_sz;
  }

  void send_to_server(rpc_t *pkt, int sz, int quorum_id)
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
    client2server_tunnel(server, quorum_id)->send(mb);
    /*
    int e = cyclone_tx(global_dpdk_context, mb, me_queue);
    if(e) {
      BOOST_LOG_TRIVIAL(warning) << "Client failed to send to server";
    }
    */
  }


  void send_to_server_async(rpc_t *pkt, int sz, int quorum_id, int flags, async_comm_t *comm_pkt)
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

            comm_pkt->channel_seq = pkt->channel_seq;
            comm_pkt->timestamp = rtc_clock::current_time();

            if(rte_ring_sp_enqueue(to_lstnr, (void *)comm_pkt) == -ENOBUFS ){
                BOOST_LOG_TRIVIAL(fatal) << "Failed to enqueue listner queue";  
                exit(-1);
            }
			client2server_tunnel(server, quorum_id)->send(mb);
           /* 
			int e = cyclone_tx(global_dpdk_context, mb, me_aqueue);
            if(e) 
                BOOST_LOG_TRIVIAL(warning) << "Client failed to send async request to server";
		   */
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
    int resp_sz = common_receive_loop(sizeof(rpc_t), 0);
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
      resp_sz = common_receive_loop(sizeof(rpc_t) + sizeof(cfg_change_t), 0);
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
      resp_sz = common_receive_loop(sizeof(rpc_t) + sizeof(cfg_change_t), 0);
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
      
	packet_out->payload_sz = sz;
	memcpy(packet_out + 1, payload, sz);
	send_to_server(packet_out, sizeof(rpc_t) + sz, quorum_id);
	resp_sz = common_receive_loop(sizeof(rpc_t) + sz, quorum_id);
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


void add_inflight()
{
    __sync_fetch_and_add(&msg_inflight, 1); 
}

void sub_inflight()
{
    __sync_fetch_and_sub(&msg_inflight, 1); 
}

int64_t get_inflight()
{
    return msg_inflight;
}







  int make_rpc_async(void *payload, int sz, 
			void (*cb)(void *, int, unsigned long), 
			void *cb_args, 
			unsigned long core_mask, int flags)
  {
	  int retcode;
	  int resp_sz;
	  int quorum_id = choose_quorum(core_mask);

	  //admission control
	  if(get_inflight() >= max_inflight){
		//BOOST_LOG_TRIVIAL(info)<< "max inflight reached  : " << get_inflight();
		return EMAX_INFLIGHT;
	  }
      // Make request
      packet_out->code        = RPC_REQ;
      packet_out->flags       = flags;
      packet_out->core_mask   = core_mask;
      packet_out->client_port = me_aqueue;
      packet_out->channel_seq = channel_aseq++;
      packet_out->client_id   = me;
      packet_out->requestor   = me_mc;
	  packet_out->payload_sz  = sz;
	  packet_out->timestamp	  = rtc_clock::current_time();
	  //packet_out->cb		  = cb;
	  memcpy(packet_out + 1, payload, sz);

	  /* populate comm_ring structure */
	  async_comm_t *comm_pkt = (async_comm_t *)rte_malloc("comm.msg",sizeof(async_comm_t),0);
	  if(comm_pkt == NULL){
			BOOST_LOG_TRIVIAL(fatal) << "Failed rpc_t allocate";
	  }
	  comm_pkt->cb = cb;
	  comm_pkt->cb_args = cb_args;

	  send_to_server_async(packet_out, sizeof(rpc_t) + sz, quorum_id, ASYNC_REQUEST, comm_pkt);
	  add_inflight();
			//	get_inflight();
			//	BOOST_LOG_TRIVIAL(info)<< "async message sent, seq no : " 
			//		<<std::to_string(packet_out->channel_seq) << " queue : "
			//		<<std::to_string(packet_out->client_port)
			//		<<"inflight msgs : " << std::to_string(get_inflight());
	 return 0;
  }

} rpc_client_t;

/* rx loop of async client */
typedef struct async_lstener_t_{
	rpc_client_t *clnt; 
	int server;
	int quorum;


int exec(){
	unsigned long mark = rte_get_tsc_cycles();
	double tsc_mhz = (rte_get_tsc_hz()/1000000.0);
	unsigned long PERIODICITY_CYCLES = PERIODICITY*tsc_mhz;
	unsigned long LOOP_TO_CYCLES     = timeout_msec * tsc_mhz;
	unsigned long elapsed_time;
	unsigned long me_aseq = 0;
	int32_t ret,available;
	rte_mbuf *pkt_array[PKT_BURST];
	rte_mbuf *m = NULL;
	async_comm_t *lookedup_m = NULL;
	async_comm_t *cur_m = NULL;
	std::map<unsigned long, async_comm_t *>::iterator it;


	//The server and quorum should be set/updated based on probing step
	server = 0;
	quorum = 0;

	// the receive port is fixed for a session. 
	int epoll_fd= epoll_create(100);
		if(epoll_fd == -1){
			BOOST_LOG_TRIVIAL(fatal) << "Unable to create epoll fd";
			exit(-1);
		}
		struct epoll_event event;
		struct epoll_event events[1];

		BOOST_LOG_TRIVIAL(info) << "Async listener "
					<< "server : " << clnt->server
					<< " quorum : " << quorum;
		tunnel_t *tun = clnt->client2server_tunnel(clnt->server, quorum);

		event.data.u32 = 0;
		event.events = EPOLLIN;
		if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tun->socket_rcv, &event) != 0){
			BOOST_LOG_TRIVIAL(fatal) << "Add epoll fd failed for client";
			exit(-1);
		}


	for(; ;){
		/* store, sent request contexts in our lookup structure */
		while(!rte_ring_sc_dequeue(clnt->to_lstnr,(void **)&cur_m)){
			//BOOST_LOG_TRIVIAL(info) << "Message sent : " << std::to_string(cur_m->channel_seq);
			clnt->pendresponse_map->insert(std::pair<unsigned long, async_comm_t *>(cur_m->channel_seq,cur_m));
		}

		/* process received message */	
		/*available = cyclone_rx_burst(0,
				   clnt->me_aqueue,
				   &pkt_array[0],
				   PKT_BURST);*/
		

		int seen_events = epoll_wait(epoll_fd, &events[0], 1,0);

		int available = 0;
		if(seen_events && tun->receive()){ // we don't go through seen indexes. We only have one port to monitor
			rte_mbuf *mb = rte_pktmbuf_alloc(global_dpdk_context->mempools[clnt->me_queue]);
			if(mb == NULL){
				BOOST_LOG_TRIVIAL(fatal) << "no mbufs for receive async client";
				exit(-1);
			}		
			tun->copy_out(mb);
			pkt_array[available++] = mb;
		}	

		if(available ){
			//BOOST_LOG_TRIVIAL(info) << "received " << available << "messages";
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
			rpc_t *resp = (rpc_t *)rte_pktmbuf_mtod_offset(m, void *, payload_offset);
			int msg_size = m->data_len - payload_offset;

			//BOOST_LOG_TRIVIAL(info) << "Response received : " << std::to_string(resp->channel_seq);
			it = clnt->pendresponse_map->find(resp->channel_seq);
			if(it != clnt->pendresponse_map->end()){	
				lookedup_m = it->second;
				//BOOST_LOG_TRIVIAL(warning) << "we have a message";
				lookedup_m->cb(lookedup_m->cb_args, resp->code == RPC_REP_OK? REP_SUCCESS:REP_FAILED,
				rtc_clock::current_time() - lookedup_m->timestamp);
				clnt->pendresponse_map->erase(it);	
				rte_free(lookedup_m);
				rte_pktmbuf_free(m);
				clnt->sub_inflight();
				__sync_synchronize();
			}else{
				// drop the packet	
				//BOOST_LOG_TRIVIAL(info) << "Message not found  : " << std::to_string(resp->channel_seq);
				rte_pktmbuf_free(m);
			}	
			}
		}
			/* check the pendingresponse_map for timeouts. We travers an ordered map. The ordering can go wrong
			 * when the seq get wrapped around at the ulong_max. But, there is no correctness issue and we are 
			 * unlikely hit it as a performance issue during benchmark runs */
			for(it = clnt->pendresponse_map->begin(); it != clnt->pendresponse_map->end(); it++){	
				if(rtc_clock::current_time() - it->second->timestamp >= async_timeout_msec){
						//BOOST_LOG_TRIVIAL(info) << "Message removed : " << it->first;
						it->second->cb(it->second->cb_args, REP_TIMEDOUT,async_timeout_msec); 
						rte_free(it->second);
						clnt->pendresponse_map->erase(it);
						clnt->sub_inflight();
						__sync_synchronize();
				}
			}
	}
}
}async_lstener_t;



int async_listener(void *arg){
	rte_cpuset_t set;
	rte_thread_get_affinity(&set);
	//BOOST_LOG_TRIVIAL(info) << "Listener thread launch, affinity = "
	//									<< get_cpuset(&set);
	async_lstener_t *lstnr = new async_lstener_t();
	lstnr->clnt = (rpc_client_t *) arg;
	lstnr->exec();
	return 0;
}

/* launch async rx/tx queues */
void cyclone_launch_clients(void *handle ,int (*f)(void *), void* arg, unsigned slave_id)
{           //first launch rx
            int e = rte_eal_remote_launch(async_listener, handle, slave_id+1);
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




void* cyclone_client_init(int client_id,
			  int client_mc,
			  int client_queue,
			  const char *config_cluster,
			  int server_ports,
			  const char *config_quorum,unsigned int flags,unsigned int max_inflight)
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
        client->me_aqueue = client_queue+1;
        client->msg_inflight = 0;
        //client->max_inflight = 1 << max_inflight;
        client->max_inflight = max_inflight;
        BOOST_LOG_TRIVIAL(info) << "max async msg in flight is set to : " 
            << std::to_string(client->max_inflight);
        //initialize comm rings for listerner thread interation
        char ringname[50];
        snprintf(ringname,50,"TO_LSTNR");
        client->to_lstnr = rte_ring_create(ringname,
                             65536,
                             rte_socket_id(),
                             RING_F_SC_DEQ);
        client->pendresponse_map = new std::map<unsigned long, async_comm_t *>();
        /* struct rte_hash_parameters hash_params = {
            .name = "asynclookup",
            .entries = UINT16_MAX,
            .reserved = 0,
            .key_len = sizeof(unsigned long),
            .hash_func = rte_hash_crc,  
            .hash_func_init_val = 0,
        };
        client->rcvmsg_tbl= rte_hash_create(&hash_params);
        if(client->rcvmsg_tbl == NULL){
            rte_exit(EXIT_FAILURE,"error creating async message receive hash table");
        } */
    }


  client->channel_seq = client_queue*client_mc*rtc_clock::current_time();
  client->client2server_tunnels = (tunnel_t *)malloc(client->replicas*num_quorums*sizeof(tunnel_t)); 
  for(int i=0;i<client->replicas;i++) {
    for(int j=0;j<num_quorums;j++) {
      client->client2server_tunnel(i, j)->init();
      client_connect_server(client_id,
			    i, 
			    j, 
			    client->client2server_tunnel(i, j));
    }
  }
  BOOST_LOG_TRIVIAL(info) << "Connections done... ";
  return (void *)client;
}

void cyclone_client_post_init(void *handle)
{
  rpc_client_t *client = (rpc_client_t *)handle;
  for(int i=0;i<num_quorums;i++) {
    client->server = 0;
    client->update_server("Initialization");
  }
}




int make_rpc_async(void *handle, 
		void *payload, 
		int sz, 
		void (*cb)(void *, int, unsigned long), 
		void *cb_args,
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
  return client->make_rpc_async(payload, sz, cb, cb_args, core_mask, flags);
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
