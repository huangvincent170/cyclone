// Setup a TCP/IP tunnel for dpdk
#ifndef TCP_TUNNEL
#define TCP_TUNNEL
#include "logging.hpp"
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "clock.hpp"

// Settings
const int MSG_MAX = 4096;

typedef struct tunnel_st {
  int msg_sz;
  char *msg;
  char *fragment;
  int socket_snd;
  int socket_rcv;

  int ready()
  {
    int cur_msg_size = *(int *)msg;
    if(msg_sz > 0 && cur_msg_size <= msg_sz) {
      return 1;
    }
    else {
       return 0;
    }
  }

  void init()
  {
    msg = (char *)malloc(MSG_MAX);
    if(msg == NULL) {
      BOOST_LOG_TRIVIAL(fatal) << "Unable to allocate msg buffer" ;
      exit(-1);
    }
    fragment = (char *)malloc(MSG_MAX);
    if(fragment == NULL) {
      BOOST_LOG_TRIVIAL(fatal) << "Unable to allocate fragment buffer" ;
      exit(-1);
    }
    msg_sz = 0;
  }
  
  int receive()
  {
    if(ready()) {
      return 1;
    }
    int bytes = recv(socket_rcv, 
		     fragment, 
		     MSG_MAX/2,
		     MSG_DONTWAIT);
    if(bytes > 0) {
      memcpy(msg + msg_sz, fragment, bytes);
      msg_sz += bytes;
    }
    return ready();
  }
 
  int receive_timeout(unsigned long timeout_usecs)
  {
    unsigned long mark = rtc_clock::current_time();
    while(!receive()) {
      if((rtc_clock::current_time() - mark)  >= timeout_usecs) {
	return 0;
      }
    }
    return 1;
  } 
 
  // Note: must only call when ready
  void copy_out(rte_mbuf *pkt)
  {
    int bytes = *(int *)msg - sizeof(int);
    memcpy(rte_pktmbuf_mtod(pkt, void *),
	   msg + sizeof(int), 
	   bytes);
    pkt->pkt_len = bytes;
    pkt->data_len = pkt->pkt_len;
    int cur_msg_size = *(int *)msg;
    if(msg_sz > cur_msg_size) {
      memcpy(msg, msg + cur_msg_size, msg_sz - cur_msg_size);
      msg_sz -= cur_msg_size;
    }
    else {
      msg_sz = 0;
    }
  }

  void copy_out_buf(char *buf)
  {
    int bytes = *(int *)msg - sizeof(int);
    memcpy(buf,
	   msg + sizeof(int), 
	   bytes);
    int cur_msg_size = *(int *)msg;
    if(msg_sz > cur_msg_size) {
      memcpy(msg, msg + cur_msg_size, msg_sz - cur_msg_size);
      msg_sz -= cur_msg_size;
    }
    else {
      msg_sz = 0;
    }
  }

  void send(rte_mbuf *pkt)
  {
    int bytes = sizeof(int);
    rte_mbuf *pkt_saved = pkt;
    while(pkt) {
      memcpy(fragment + bytes, 
	     rte_pktmbuf_mtod(pkt, void*),
	     pkt->data_len);
      bytes += pkt->data_len;
      pkt    = pkt->next;
    }

    *(int *)fragment = bytes;
    char * buf =fragment;
    while(bytes) {
      int bytes_sent = ::send(socket_snd, buf, bytes, 0);
      if(bytes_sent > 0) {
	bytes -= bytes_sent;
	buf   += bytes_sent;
      }
    }
    rte_pktmbuf_free(pkt_saved);
  }

  void send_buf(void *data, int size)
  {
    int bytes = sizeof(int);
    memcpy(fragment + bytes, 
	   data,
	   size);
    bytes += size;
    *(int *)fragment = bytes;
    char * buf =fragment;
    while(bytes) {
      int bytes_sent = ::send(socket_snd, buf, bytes, 0);
      if(bytes_sent > 0) {
	bytes -= bytes_sent;
	buf   += bytes_sent;
      }
    }
  }

}tunnel_t;

extern tunnel_t *server2server_tunnels;
extern tunnel_t **server2client_tunnels;
extern tunnel_t* server2server_tunnel(int server, int quorum);
extern tunnel_t* server2client_tunnel(int client, int quorum); 
extern tunnel_t* client2server_tunnel(int server);
extern void server_open_ports(int me, int quorum);
extern void server_accept_server(int socket,
				 int quorum, 
				 int replicas);
void server_accept_client(int socket, int quorum);

extern void server_connect_server(int quorum,
				  int me,
				  int replicas);
void client_connect_server(int client_id, int replica, int quorum, tunnel_t *tun);
extern sockaddr_in *server_addresses;
extern sockaddr_in *client_addresses;
extern int *sockets_raft;
extern int *sockets_client;
// Random fixed port numbers
const int PORT_SERVER_BASE = 0xf16;

#endif
