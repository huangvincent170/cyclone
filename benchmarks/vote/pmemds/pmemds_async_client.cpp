#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <rte_launch.h>
#include <rte_malloc.h>

#include <vector>

#include "../../../core/logging.hpp"
#include "../../../core/clock.hpp"
#include "../../../core/libcyclone.hpp"

#include "pmemds-client.h"
#include "tree/btree-client.h"
#include "../genzipf.h"


unsigned long pmemds_keys = 1000;
static const unsigned int write_every = 20; /* 95% reads */
unsigned int w_every_cnt = 0;


/* pmem structure names */
static const uint16_t ART_MAP = 0;
static const uint16_t VOTE_MAP = 1;


int driver(void *arg);


bool is_update(){
    if(w_every_cnt == (write_every-1)){
        ++w_every_cnt%write_every;
        return true;
    }
    w_every_cnt++;
    return false;
}

typedef struct driver_args_st
{
    int leader;
    int me;
    int mc;
    int replicas;
    int clients;
    int partitions;
    int buf_cap;
    pmemdsclient::DPDKPMClient *dpdkClient;
    pmemdsclient::HashmapEngine *art_map;
    pmemdsclient::HashmapEngine *vote_map;
    void **handles;
    void operator() ()
    {
        (void)driver((void *)this);
    }
} driver_args_t;

int driver(void *arg)
{
    unsigned long request_id = 0UL; // wrap around at MAX
    driver_args_t *dargs = (driver_args_t *)arg;
    int me = dargs->me;
    int mc = dargs->mc;
    int replicas = dargs->replicas;
    int clients = dargs->clients;
    int partitions = dargs->partitions;
    void **handles = dargs->handles;

    pmemdsclient::DPDKPMClient *pmlib = dargs->dpdkClient;
    pmemdsclient::HashmapEngine *art_map = dargs->art_map;
    pmemdsclient::HashmapEngine *vote_map = dargs->art_map;

    //struct proposal *prop = (struct proposal *)buffer;
    srand(time(NULL));
    int ret;
    int rpc_flags;
    int my_core;



    unsigned  long key;
    char value_buffer[64];


    BOOST_LOG_TRIVIAL(info) << "FRAC_READ = " << ((write_every-1) * 100.00)/write_every;

    unsigned long keys = pmemds_keys;
    const char *keys_env = getenv("KV_KEYS");
    if (keys_env != NULL)
    {
        keys = atol(keys_env);
    }
    BOOST_LOG_TRIVIAL(info) << "KEYS = " << keys;
    srand(rtc_clock::current_time());

    pmlib->open("voteApp",nullptr);
    uint8_t creation_flag = 0;
    /* create two holding structures for the app */
    art_map->create(creation_flag,nullptr);
    vote_map->create(creation_flag,nullptr);

    /* populate article data */

    for(unsigned long i; i < pmemds_keys; i++){
        snprintf(value_buffer,64,"article_%lu, http://",key);
        art_map->put_art(i,value_buffer, nullptr);
    }

    for( ; ; ){
        unsigned long art_id = zipf(1.08,pmemds_keys);
        if (is_update()){
            snprintf(value_buffer,64,"%lu",key);
            BOOST_LOG_TRIVIAL(info) << "vote article :" << key;
            pmlib->vote_up(art_id,nullptr);
        }
        else{
            BOOST_LOG_TRIVIAL(info) << "get article :" << key;
            btree->get_art(art_id, nullptr);
        }
    }
    btree->close(nullptr);
    pmlib->close(nullptr);
    return 0;
}

int main(int argc, const char *argv[])
{
    if (argc != 11)
    {
        printf("Usage: %s client_id_start client_id_stop mc replicas clients partitions cluster_config quorum_config_prefix server_ports inflight_cap\n", argv[0]);
        exit(-1);
    }

    int client_id_start = atoi(argv[1]);
    int client_id_stop  = atoi(argv[2]);
    driver_args_t *dargs;
    void **prev_handles;
    cyclone_network_init(argv[7], 1, atoi(argv[3]), 2 + client_id_stop - client_id_start);
    driver_args_t ** dargs_array =
            (driver_args_t **)malloc((client_id_stop - client_id_start) * sizeof(driver_args_t *));
    //BOOST_LOG_TRIVIAL(info) << "no. clients: " << (client_id_stop - client_id_start);
    for (int me = client_id_start; me < client_id_stop; me++)
    {
        dargs = (driver_args_t *) malloc(sizeof(driver_args_t));
        dargs_array[me - client_id_start] = dargs;
        if (me == client_id_start)
        {
            dargs->leader = 1;
        }
        else
        {
            dargs->leader = 0;
        }
        dargs->me = me;
        dargs->mc = atoi(argv[3]);
        dargs->replicas = atoi(argv[4]);
        dargs->clients  = atoi(argv[5]);
        dargs->partitions = atoi(argv[6]);
        dargs->buf_cap = atoi(argv[10]);
        dargs->handles = new void *[dargs->partitions];
        BOOST_LOG_TRIVIAL(info) << "no. of partitions: " << dargs->partitions;
        char fname_server[50];
        char fname_client[50];
        for (int i = 0; i < dargs->partitions; i++){
            sprintf(fname_server, "%s", argv[7]);
            sprintf(fname_client, "%s%d.ini", argv[8], i); // only one raft instance running
            dargs->handles[i] = cyclone_client_init(dargs->me,
                                                    dargs->mc,
                                                    1 + me - client_id_start,
                                                    fname_server,
                                                    atoi(argv[9]),
                                                    fname_client,
                                                    CLIENT_ASYNC,
                                                    dargs->buf_cap);
            BOOST_LOG_TRIVIAL(info) << "init dpdkclient and btree";
            dargs->dpdkClient = new pmemdsclient::DPDKPMClient(dargs->handles[i]);
            dargs->art_map = new pmemdsclient::HashMapEngine(dargs->dpdkClient,ART_MAP,100,1UL);
            dargs->vote_map = new pmemdsclient::HashMapEngine(dargs->dpdkClient,VOTE_MAP,100,1UL);
        }
    }
    for (int me = client_id_start; me < client_id_stop; me++){
        cyclone_launch_clients(dargs_array[me-client_id_start]->handles[0],driver, dargs_array[me-client_id_start], 1+  me-client_id_start);
    }
    rte_eal_mp_wait_lcore();
}
