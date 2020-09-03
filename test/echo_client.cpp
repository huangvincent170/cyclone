/*
 * Copyright (c) 2015, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

int driver(void *arg);

typedef struct driver_args_st {
  int leader;
  int me; 
  int mc;
  int replicas;
  int clients;
  int partitions;
  void **handles;
  void operator() ()
  {
    (void)driver((void *)this);
  }
} driver_args_t;

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
  int sz;
  struct proposal *resp;
  unsigned long order = 0;
  unsigned long tx_block_cnt   = 0;
  unsigned long tx_block_begin = rtc_clock::current_time();
  unsigned long total_latency  = 0;
  int rpc_flags;

  int my_core;
  
  unsigned long payload = 0;
  const char *payload_env = getenv("PAYLOAD");
  if(payload_env != NULL) {
    payload = atol(payload_env);
  }
  BOOST_LOG_TRIVIAL(info) << "PAYLOAD = " << payload;
  cyclone_client_post_init(handles[0]);
  total_latency = 0;
  tx_block_cnt  = 0;
  tx_block_begin = rtc_clock::current_time();
  unsigned long tx_begin_time = rtc_clock::current_time();
  srand(tx_begin_time);
  int partition;
  while(true) {
    rpc_flags = 0;
    //rpc_flags = RPC_FLAG_RO;
    my_core = dargs->me % executor_threads;
    //BOOST_LOG_TRIVIAL(info) << "request loop";
    sz = make_rpc(handles[0],
		  buffer,
		  payload,
		  (void **)&resp,
		  1UL << my_core,
		  rpc_flags);
    if(sz != payload) {
      BOOST_LOG_TRIVIAL(fatal) << "Invalid response";
    }
    tx_block_cnt++;
    
    if(dargs->leader) {
      if(tx_block_cnt > 5000) {
	total_latency = (rtc_clock::current_time() - tx_begin_time);
	BOOST_LOG_TRIVIAL(info) << "LOAD = "
				<< ((double)1000000*tx_block_cnt)/total_latency
				<< " tx/sec "
				<< "LATENCY = "
				<< ((double)total_latency)/tx_block_cnt
				<< " us ";
	tx_begin_time = rtc_clock::current_time();
	tx_block_cnt   = 0;
	total_latency  = 0;
      }
    }
  }
  return 0;
}

int main(int argc, const char *argv[]) {
  if(argc != 10) {
    printf("Usage: %s client_id_start client_id_stop mc replicas clients partitions cluster_config quorum_config_prefix server_ports\n", argv[0]);
    exit(-1);
  }

  ///////////////////////////////////////////
  char key[150];
  boost::property_tree::ptree pt_cluster;
  boost::property_tree::read_ini(argv[7], pt_cluster);

  server_addresses      = (struct sockaddr_in *)malloc(3*sizeof(struct sockaddr_in));
  /* server addresses for tunnel */
  //for(int i=0;i<3;i++) { // FIX_IT
  for(int i=0;i<1;i++) {
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
					      CLIENT_SYNC,
					      0);
    }
  }
  BOOST_LOG_TRIVIAL(info) << "sleep 10 sec...";
  sleep(10);
  for(int me = client_id_start; me < client_id_stop; me++) {
    int e = rte_eal_remote_launch(driver, dargs_array[me-client_id_start], 1 + me - client_id_start);
    if(e != 0) {
      BOOST_LOG_TRIVIAL(fatal) << "Failed to launch driver on remote lcore";
      exit(-1);
    }
  }
  rte_eal_mp_wait_lcore();
}
