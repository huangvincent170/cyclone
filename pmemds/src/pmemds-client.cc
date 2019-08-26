
#include <iostream>

#include "clock.hpp"
#include "pmemds-client.h"

namespace pmemdsclient {
    int PMClient::open(const std::string &appname) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,OPEN);
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",appname.c_str());
        if(!sendmsg(&payload,&response,1UL)){
            LOG_ERROR("pmclient open");
        }
        if(STATUS(response->meta) == OK){
            return OK;
        }
        LOG_ERROR("application open");
        return FAILED;
    }

    int PMClient::open(const std::string &appname,void (*cb)(void *)) {
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
        if(!sendmsg(&payload,&response,1UL)){
            LOG_ERROR("pmclient close");
        }
        if(STATUS(response->meta) == OK){
            return OK;
        }
        LOG_ERROR("application close");
        return FAILED;
    }

    int PMClient::close(void (*cb)(void *)) {
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,CLOSE);
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",this->appname.c_str());
        if (sendmsg_async(&payload, 1UL, cb) != 0) {
            LOG_ERROR("pmclient close");
        }
        return OK;
    }


    int PMClient::topk() {
        pm_rpc_t *response;
        int resp_size;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,GET_TOPK);
        if(!(resp_size = sendmsg(&payload,&response,1UL))){
            LOG_ERROR("get topK articles");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("priority queue getmax");
            return FAILED;
        }

        //figure out variable length data size
        int var_length = resp_size - (sizeof(pm_rpc_t) - sizeof(response->value));
        assert(var_length%sizeof(struct vote_payload_st) == 0 && "var length error");
        int array_size = var_length/sizeof(struct vote_payload_st);
        struct vote_payload_st *var_array = (struct vote_payload_st *)response->value;
        for(int i = 0; i < array_size; i++){
            std::cout << var_array[i].idx << " " << std::string(var_array[i].art) << std::endl;
        }
        //max = response->key;
        return OK;
    }


    int PMClient::topk(void (*cb)(void *)) {
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,GET_TOPK);
        if (sendmsg_async(&payload, 1UL, cb) != 0) {
            LOG_ERROR("btree async put");
        }
        return OK;
    }
}