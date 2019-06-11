
#include <iostream>
#include <rte_malloc.h>

#include "clock.hpp"
#include "pmemds-client.h"


namespace pmemdsclient {

    int PMClient::open(const std::string &appname) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,OPEN);
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",appname.c_str());
        if(sendmsg(&payload,&response,1UL) != 0){
            LOG_ERROR("pmclient open");
        }
        if(response->meta == OK){
            return OK;
        }
        LOG_ERROR("application open");
        return FAILED;
    }

    int PMClient::open(const std::string &appname,void (*cb)(void *)) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,OPEN);
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",appname.c_str());
        if (sendmsg_async(&payload,1UL, cb) != 0) {
            LOG_ERROR("pmclient open");
        }
        return OK;
    }

    int PMClient::close() {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,CLOSE);
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",this->appname.c_str());
        if(sendmsg(&payload,&response,1UL) != 0){
            LOG_ERROR("pmclient close");
        }
        if(response->meta == OK){
            return OK;
        }
        LOG_ERROR("application close");
        return FAILED;
    }

    int PMClient::close(void (*cb)(void *)) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,CLOSE);
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",this->appname.c_str());
        if (sendmsg_async(&payload, 1UL, cb) != 0) {
            LOG_ERROR("pmclient close");
        }
        return OK;
    }
}

#include "libcyclone.hpp"


unsigned long tx_wr_block_cnt = 0UL;
unsigned long tx_ro_block_cnt = 0UL;
unsigned long tx_failed_cnt = 0UL;
unsigned long total_latency = 0UL;
unsigned long tx_begin_time = 0UL;
volatile int64_t timedout_msgs = 0;


/* callback context */
typedef struct cb_st
{
    uint8_t request_type;
    unsigned long request_id;
    void (*callback)(void *);
    pm_rpc_t *response;
} cb_t;


/* measurement handler */
void async_callback(void *args, int code, unsigned long msg_latency)
{
    struct cb_st *cb_ctxt = (struct cb_st *)args;
    if(code == REP_SUCCESS){
        //BOOST_LOG_TRIVIAL(info) << "received message " << cb_ctxt->request_id;
        cb_ctxt->request_type == UPDATE_OPERATION ? tx_wr_block_cnt++ : tx_ro_block_cnt++;
        rte_free(cb_ctxt);
    }else{
		std::cout << "timed-out message " << cb_ctxt->request_id << std::endl;
        exit(-1);

    }

    total_latency += msg_latency;
    if ((tx_wr_block_cnt + tx_ro_block_cnt) >= 5000)
    {
        unsigned long total_elapsed_time = (rtc_clock::current_time() - tx_begin_time);
		std::cout << "LOAD = "
                                << ((double)1000000 * (tx_wr_block_cnt + tx_ro_block_cnt)) / total_elapsed_time
                                << " tx/sec "
                                << "LATENCY = "
                                << ((double)total_latency) / (tx_wr_block_cnt + tx_ro_block_cnt)
                                << " us "
                                << "wr/rd = "
                                <<((double)tx_wr_block_cnt/tx_ro_block_cnt)
								<< std::endl;
        tx_wr_block_cnt   = 0;
        tx_ro_block_cnt   = 0;
        tx_failed_cnt  = 0;
        total_latency  = 0;
        tx_begin_time = rtc_clock::current_time();
    }
}





namespace pmemdsclient{

    DPDKPMClient::DPDKPMClient(void *clnt){
		this->dpdk_client = clnt;
        this->request_id = 0UL;
    }

    DPDKPMClient::~DPDKPMClient() {}


    int DPDKPMClient::sendmsg(pm_rpc_t* msg , pm_rpc_t** response,unsigned long core_mask) {
        return make_rpc(this->dpdk_client,msg,sizeof(pm_rpc_t),
                 reinterpret_cast<void **>(response),core_mask,0);
    }

    int DPDKPMClient::sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *)) {

        struct cb_st *cb_ctxt = (struct cb_st *)rte_malloc("callback_ctxt", sizeof(struct cb_st), 0);
        //cb_ctxt->request_type = rpc_flags;
        cb_ctxt->request_id = request_id++;
        cb_ctxt->callback = cb;
        int rpc_flags = 0;
        int ret;
        do{
            ret = make_rpc_async(this->dpdk_client,
                                 msg,
                                 sizeof(pm_rpc_t),
                                 async_callback,
                                 (void *)cb_ctxt,
                                 core_mask,
                                 rpc_flags);
            if(ret == EMAX_INFLIGHT){
                //sleep a bit
                continue;
            }
        }while(ret);
        return 0;
    }




}
