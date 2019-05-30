#include<assert.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <time.h>
#include<unistd.h>

#include "../core/libcyclone.hpp"
#include "../core/logging.hpp"
#include "../core/clock.hpp"
#include "../../../pmemds/include/tree/btree-client.h"

static void *logs[executor_threads];
pmemds::PMLib *lib;



void callback(const unsigned char *data,
              const int len,
              rpc_cookie_t *cookie) {

    pm_rpc_t *req = (pm_rpc_t *) data;

    cookie->ret_size = sizeof(pm_rpc_t); // TODO: rocksdb code uses len
    cookie->ret_value = malloc(sizeof(pm_rpc_t));
    pm_rpc_t *res = (pm_rpc_t *) cookie->ret_value;
    lib->exec(req, res);
}

int wal_callback(const unsigned char *data,
                 const int len,
                 rpc_cookie_t *cookie)
{
    return cookie->log_idx;
}


void gc(rpc_cookie_t *cookie)
{
    free(cookie->ret_value);
}

rpc_callbacks_t rpc_callbacks =
        {
                callback,
                gc,
                wal_callback
        };



int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        printf("Usage1: %s replica_id replica_mc clients cluster_config quorum_config ports\n", argv[0]);
        exit(-1);
    }

    int server_id = atoi(argv[1]);
    cyclone_network_init(argv[4],
                         atoi(argv[6]),
                         atoi(argv[2]),
                         atoi(argv[6]) + num_queues * num_quorums + executor_threads);


    lib = new pmemds::PMLib(); //create PMLib instance

    dispatcher_start(argv[4],
                     argv[5],
                     &rpc_callbacks,
                     server_id,
                     atoi(argv[2]),
                     atoi(argv[3]));

}

