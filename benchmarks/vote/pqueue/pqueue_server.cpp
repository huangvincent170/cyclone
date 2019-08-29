#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "../../../core/libcyclone.hpp"
#include "../../../core/logging.hpp"
#include "../../../core/clock.hpp"
#include "libpmemobj.h"


#include "pmemds.h"

static const std::string pmem_path = "/mnt/pmem1/pmemds";
pmemds::PMLib *pmlib;

void callback(uint8_t thread_id, const unsigned char *data,
              const int len,
              rpc_cookie_t *cookie, unsigned long *pmdk_state)
{
	pm_rpc_t *request, *response;
	/* set mbuf/wal commit state as a thread local */
	//TX_SET_BLIZZARD_MBUF_COMMIT_ADDR(pmdk_state);
    request = (pm_rpc_t *) data;
    pmlib->exec(thread_id, request,&response,&(cookie->ret_size));
    cookie->ret_value = (void *)response;
}
/*commute rules
 * 1. update operation going in to different partitions commute
 * 2. Read operations commute with each other irrespective of the partition
 * 3. Others do not
 */
int commute_callback(unsigned long cmask1, void *arg1, unsigned long cmask2, void *arg2)
{
	pm_rpc_t *op1 = (pm_rpc_t *) arg1;
	pm_rpc_t *op2 = (pm_rpc_t *) arg2;

	unsigned int op1_id = OP_ID(op1->meta);
	unsigned int op2_id = OP_ID(op2->meta);
	if((op1_id == PUT || op1_id == INSERT) &&
			(op2_id == PUT || op2_id == INSERT) &&
				(cmask1 != cmask2)){
		return 1;
	}
	if(op1_id == GET_TOPK && op2_id == GET_TOPK){
		return 1;
	}
	return 0;
}

/*
 * commute scheduler uses return value to decide the the core.
 * Alternatively, the client can specify core_mask. But this
 * way, client is fairly transparent to the existence of partitioned data-structure
 * 1. update operation's partitions is determined by modulo hash.
 * 2. read operations go to 0 partition TBD -- make it round robin
 */
int partition_callback(void *request){
	pm_rpc_t *op = (pm_rpc_t *) request;
	int partition = 0;
	unsigned int op_id = OP_ID(op->meta);
	if(op_id == PUT || op_id == INSERT){
		partition = op->key%executor_threads;	
		return partition;
	}
	return partition; //defualt partition
}

void gc(rpc_cookie_t *cookie)
{
    free(cookie->ret_value);
}

rpc_callbacks_t rpc_callbacks = { callback,
								  gc,
								  commute_callback,
								  partition_callback
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
    pmlib = new pmemds::PMLib(pmem_path,executor_threads);
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

