#include <string>

#include "btree-client.h"


namespace pmemdsclient {

    BTreeEngine::BTreeEngine(PMClient *handle, const std::string &ds_name, size_t size, unsigned long core_mask)
            :client(handle),ds_name(ds_name),core_mask(core_mask) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,CREATE);
        SET_TYPE_ID(payload.meta,SORTED_BTREE);
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",ds_name.c_str());
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("btree create");
        }
        if(response->meta != OK){
            LOG_ERROR("btree create");
            exit(1);
        }
    }


    BTreeEngine::~BTreeEngine() {

        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,DELETE);
        SET_TYPE_ID(payload.meta,SORTED_BTREE);
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",this->ds_name.c_str());
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("btree delete");
        }
        if(response->meta != OK){
            LOG_ERROR("btree delete");
            exit(1);
        }
    }

    std::string BTreeEngine::get(const unsigned long key) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_DS_ID(payload.meta, this->ds_id);
        SET_OP_ID(payload.meta,GET);
        SET_TYPE_ID(payload.meta,SORTED_BTREE);
        payload.key = key;
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("get operation");
        }
        if(response->meta != OK){
            return nullptr;
        }
        std::string ret_string(response->value,MAX_VAL_LENGTH);
        return ret_string;
    }

    int BTreeEngine::put(const unsigned long key, const std::string& value) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_DS_ID(payload.meta, this->ds_id);
        SET_OP_ID(payload.meta,PUT);
        SET_TYPE_ID(payload.meta,SORTED_BTREE);
        payload.key = key;
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",value.c_str());
        client->sendmsg(&payload,&response,this->core_mask);
        if(response->meta != OK){
            LOG_ERROR("btree put");
            return FAILED;
        }
        return OK;
    }
}