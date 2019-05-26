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
}

#include "libcyclone.hpp"


namespace pmemdsclient{

    DPDKPMClient::DPDKPMClient(void *clnt){

    }

    DPDKPMClient::~DPDKPMClient() {}


    int DPDKPMClient::sendmsg(pm_rpc_t* msg , pm_rpc_t** response,unsigned long core_mask) {
        /*return make_rpc(this->dpdk_client,msg,sizeof(pm_rpc_t),
                 reinterpret_cast<void **>(response),core_mask,RPC_FLAG_RO);*/
    }

    int DPDKPMClient::sendmsg_async(pm_rpc_t *msg, void (*cb)(void *, int, unsigned long)) {
        return 1;
    }


    TestClient::TestClient(pmemds::PMLib *pmLib,pm_rpc_t *request,
                           pm_rpc_t *response):req(request),res(response),pmLib(pmLib) {

    }

    TestClient::~TestClient() {

    }

    int TestClient::sendmsg(pm_rpc_t *req, pm_rpc_t **response, unsigned long core_mask) {
        *response = new pm_rpc_t();
        pmLib->exec(req,*response);
        return 0; // no send errors
    }

    int TestClient::sendmsg_async(pm_rpc_t *msg, void (*cb)(void *, int, unsigned long)) {
        return 1;
    }



}