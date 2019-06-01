#include <string>

#include "tree/btree-client.h"


namespace pmemdsclient {

    BTreeEngine::BTreeEngine(PMClient *handle, const uint16_t ds_id, size_t size, unsigned long core_mask)
            {
                this->ds_id = ds_id;
        this->size = size;
        this->client = handle;
        this->core_mask = core_mask;
    }


    BTreeEngine::~BTreeEngine() {

    }


    int BTreeEngine::create(uint8_t flags) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,CREATE_DS);
        SET_TYPE_ID(payload.meta,SORTED_BTREE);
        SET_DS_ID(payload.meta,this->ds_id);
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("btree create");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("btree create");
            return FAILED;
        }
        return OK;
    }

    /* remove the data-structure altogether */
    int BTreeEngine::close() {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,CLOSE_DS);
        SET_TYPE_ID(payload.meta,SORTED_BTREE);
        SET_DS_ID(payload.meta,this->ds_id);
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("btree close");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("btree close");
            return FAILED;
        }
        return OK;
    }


    /* remove the data-structure altogether */
    int BTreeEngine::remove() {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,REMOVE_DS);
        SET_TYPE_ID(payload.meta,SORTED_BTREE);
        SET_DS_ID(payload.meta,this->ds_id);
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("btree remove");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("btree remove");
            return FAILED;
        }
        return OK;
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
        if(STATUS(response->meta) != OK){
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
        if(STATUS(response->meta) != OK){
            LOG_ERROR("btree put");
            return FAILED;
        }
        return OK;
    }


    int BTreeEngine::remove(const unsigned long key ) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_DS_ID(payload.meta, this->ds_id);
        SET_OP_ID(payload.meta,DELETE);
        SET_TYPE_ID(payload.meta,SORTED_BTREE);
        payload.key = key;
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("btree key delete");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("btree key delete");
            return FAILED;
        }
        return OK;
    }

}