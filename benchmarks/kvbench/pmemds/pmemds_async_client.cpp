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


unsigned long pmemds_keys = 100000000;

/* pmem structure names */
const uint16_t btreemap_st = 0;

pmemdsclient::DPDKPMClient *dpdkClient;
pmemdsclient::BTreeEngine *btreeMap;

int driver(void *arg);

typedef struct driver_args_st
{
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
    char *buffer = new char[DISP_MAX_MSGSIZE];
    //struct proposal *prop = (struct proposal *)buffer;
    srand(time(NULL));
    int ret;
    int rpc_flags;
    int my_core;

    unsigned  long key;
    char value_buffer[64];

    double frac_read = 0.5;
	const char *frac_read_env = getenv("KV_FRAC_READ");
	if (frac_read_env != NULL)
	{
		frac_read = atof(frac_read_env);
	}
	BOOST_LOG_TRIVIAL(info) << "FRAC_READ = " << frac_read;

	unsigned long keys = pmemds_keys;
	const char *keys_env = getenv("KV_KEYS");
	if (keys_env != NULL)
	{
		keys = atol(keys_env);
	}
	BOOST_LOG_TRIVIAL(info) << "KEYS = " << keys;

    pm_rpc_t *rpc_buf = (pm_rpc_t *)buffer;

    srand(rtc_clock::current_time());
    struct cb_st *cb_ctxt;

    dpdkClient->open("kvApp",null);
    for( ; ; ){
        double coin = ((double)rand()) / RAND_MAX;
		unsigned long key = rand() % keys;
        if (coin > frac_read){
            snprintf(value_buffer,64,"%lu",key);
            btreeMap->put(key,value_buffer, nullptr);
        }
        else{
            btreeMap->get(key, nullptr);
        }
    }
    dpdkClient->close(nullptr);
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

            dpdkClient = new pmemdsclient::DPDKPMClient(dargs->handles[i]);
            btreeMap = new pmemdsclient::BTreeEngine(dpdkClient,btreemap_st,100,1UL<<1);
        }
    }
    for (int me = client_id_start; me < client_id_stop; me++){
        cyclone_launch_clients(dargs_array[me-client_id_start]->handles[0],driver, dargs_array[me-client_id_start], 1+  me-client_id_start);
    }
    rte_eal_mp_wait_lcore();
}
