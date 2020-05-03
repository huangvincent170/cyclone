#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "../core/clock.hpp"
#include "../core/logging.hpp"
#include "../core/tcp_tunnel.hpp"
#include <libcyclone.hpp>
#include <rte_launch.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


unsigned long request_id = 0UL; // wrap around at MAX
std::vector<unsigned long> *replayq;
std::map<unsigned long, unsigned long> *clnt_map;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

unsigned long tx_block_cnt;
unsigned long tx_failed_cnt;
unsigned long total_latency;
unsigned long tx_begin_time;

int driver(void *arg);

typedef struct replay_st{
    unsigned long client_id;
    unsigned long latency;
}relay_t;

typedef struct driver_args_st {
  int leader;
  int me; 
  int mc;
  int replicas;
  int clients;
  int partitions;
  int buf_cap;
  void **handles;
  void operator() ()
  {
    (void)driver((void *)this);
  }
} driver_args_t;

typedef struct cb_st{
    unsigned long request_id;
}cb_t;


void async_callback(void *args, int code, unsigned long msg_latency){
    //cb_t *cb = (cb_t *)args;
    if(code == REP_SUCCESS){
        tx_block_cnt++;
    }else{
        tx_failed_cnt++;
    }
    //free(cb);
    total_latency += msg_latency; //timouts get added in to message latency
    if(tx_block_cnt >= 5000) {
        unsigned long total_elapsed_time = (rtc_clock::current_time() - tx_begin_time);
        std::cout << "LOAD = "
                << ((double)1000000*tx_block_cnt)/total_elapsed_time
                << " tx/sec "
                << "LATENCY = "
                << ((double)total_latency)/tx_block_cnt
                << " us "
                << std::endl;
        tx_block_cnt   = 0;
        tx_failed_cnt  = 0;
        total_latency  = 0;
        tx_begin_time = rtc_clock::current_time();
    }
}


int driver(void *arg)
{
   driver_args_t *dargs = (driver_args_t *)arg;
  int me = dargs->me;
  int mc = dargs->mc;
  int replicas = dargs->replicas;
  int clients = dargs->clients;
  int partitions = dargs->partitions;
  void **handles = dargs->handles;
  char *buffer = new char[DISP_MAX_MSGSIZE];
  struct proposal *prop = (struct proposal *)buffer;
  srand(time(NULL));
  struct proposal *resp;
  int rpc_flags;
    int ret;

  int my_core;
    
  total_latency = 0;
  tx_block_cnt  = 0;
  tx_begin_time = rtc_clock::current_time();
  unsigned long payload = 0;



  BOOST_LOG_TRIVIAL(info) << "PAYLOAD = " << payload;
  srand(rtc_clock::current_time());
  int partition;
  int newcb  = 1;
  for( ; ; ) { 
    rpc_flags = 0;
    my_core = dargs->me % executor_threads;
        //cb_t *cb_args = (cb_t *)malloc(sizeof(cb_t));;
        //cb_args->request_id =request_id++;
        do{
            ret = make_rpc_async(handles[0],
                buffer,
                payload,
                async_callback,
                (void *)NULL,
                1UL << my_core,
                rpc_flags);
            if(ret == EMAX_INFLIGHT) {
                //BOOST_LOG_TRIVIAL(fatal) << "buffer full";
                continue;
            }
        }while(ret);
  }
  return 0; 
 }

int main(int argc, const char *argv[]) {
  if(argc != 10) {
    printf("Usage: %s client_id_start client_id_stop mc replicas"
		   	"clients partitions cluster_config quorum_config_prefix" 
			"server_ports inflight_cap\n", argv[0]);
    exit(-1);
  }

  ///////////////////////////////////////////
  char key[150];
  boost::property_tree::ptree pt_cluster;
  boost::property_tree::read_ini(argv[7], pt_cluster);

  server_addresses      = (struct sockaddr_in *)malloc(3*sizeof(struct sockaddr_in));
  /* server addresses for tunnel */
  //for(int i=0;i<3;i++) { // FIX_IT
  for(int i=0;i<2;i++) {
    sprintf(key, "machines.ipaddr%d", i);
    std::string s = pt_cluster.get<std::string>(key);
    BOOST_LOG_TRIVIAL(info) << "Setting address " 
			    << i 
			    << " to " 
			    << s;
    if(inet_aton(s.c_str(), &server_addresses[i].sin_addr) == 0) {
      BOOST_LOG_TRIVIAL(fatal) << "Unable to convert "
			       << s;
      exit(-1);
    }
  }

  ////////////////////////////////////////////

  int client_id_start = atoi(argv[1]);
  int client_id_stop  = atoi(argv[2]);
  driver_args_t *dargs;
  void **prev_handles;
  
  cyclone_network_init(argv[7], 1, atoi(argv[3]), 1 + client_id_stop - client_id_start);
  driver_args_t ** dargs_array = 
    (driver_args_t **)malloc((client_id_stop - client_id_start)*sizeof(driver_args_t *));
  for(int me = client_id_start; me < client_id_stop; me++) {
    dargs = (driver_args_t *) malloc(sizeof(driver_args_t));
    dargs_array[me - client_id_start] = dargs;
    if(me == client_id_start) {
      dargs->leader = 1;
    }
    else {
      dargs->leader = 0;
    }
    dargs->me = me;
    dargs->mc = atoi(argv[3]);
    dargs->replicas = atoi(argv[4]);
    dargs->clients  = atoi(argv[5]);
    dargs->partitions = atoi(argv[6]);
	dargs->buf_cap = atoi(argv[10]);
    dargs->handles = new void *[dargs->partitions];
    char fname_server[50];
    char fname_client[50];
    for(int i=0;i<dargs->partitions;i++) {
      sprintf(fname_server, "%s", argv[7]);
      sprintf(fname_client, "%s%d.ini", argv[8], i);
      BOOST_LOG_TRIVIAL(info) << "Exec client init...";
      dargs->handles[i] = cyclone_client_init(dargs->me,
					      dargs->mc,
					      1 + me - client_id_start,
					      fname_server,
					      atoi(argv[9]),
					      fname_client,
						  CLIENT_ASYNC,
						  dargs->buf_cap);
    }
  }
  BOOST_LOG_TRIVIAL(info) << "sleep 10 sec...";
  sleep(10);
  for(int me = client_id_start; me < client_id_stop; me++) {
	  cyclone_launch_clients(dargs_array[me-client_id_start]->handles[0],driver, dargs_array[me-client_id_start], 1+me-client_id_start);
  }
  rte_eal_mp_wait_lcore();
}
