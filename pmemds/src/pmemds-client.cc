
#include <iostream>

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