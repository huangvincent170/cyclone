#include<assert.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <time.h>
#include<unistd.h>

#include "../../../core/libcyclone.hpp"
#include "../../../core/logging.hpp"
#include "../../../core/clock.hpp"
#include "pmemds.h"

pmemds::PMLIB *pmlib;
pmemds::pm_rpc_t response;



void callback(const unsigned char *data,
              const int len,
              rpc_cookie_t *cookie)
{
    cookie->ret_value = &response; //TODO: works only for one execution thread
    cookie->ret_size = sizeof(pmemds::pm_rpc_t);

    pmemds::pm_rpc_t *request = (pmemds::pm_rpc_t *) data;
    pmlib->exec(request,&response);

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

    assert(pmlib == NULL);
    pmlib = new pmemds::PMLIB();
    if (pmlib == nullptr)
    {
        BOOST_LOG_TRIVIAL(fatal) << "cannot open pmemds";
        exit(-1);
    }

    dispatcher_start(argv[4],
                     argv[5],
                     &rpc_callbacks,
                     server_id,
                     atoi(argv[2]),
                     atoi(argv[3]));

}

