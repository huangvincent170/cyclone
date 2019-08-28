#include <string>

#include "hashmap/hashmap-client.h"


namespace pmemdsclient {

    HashMapEngine::HashMapEngine(PMClient *handle, const uint16_t ds_id, size_t size, unsigned long core_mask) {
        this->ds_id = ds_id;
        this->size = size;
        this->client = handle;
        this->core_mask = core_mask;
        this->type_id = CONCURRENT_HASHMAP;
    }


    HashMapEngine::HashMapEngine(PMClient *handle, const uint16_t ds_id, size_t size, unsigned long core_mask, uint8_t npartitions) { // change this to a bitmap flag
        this->ds_id = ds_id;
        this->size = size;
        this->client = handle;
        this->core_mask = core_mask;
        this->type_id = SHARDED_HASHMAP;
    }

    HashMapEngine::~HashMapEngine() {

    }


    int HashMapEngine::create(uint8_t flags) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0, 0, "\0"};
        SET_OP_ID(payload.meta, CREATE_DS);
        SET_TYPE_ID(payload.meta, this->type_id);
        SET_DS_ID(payload.meta, this->ds_id);
        if (!client->sendmsg(&payload, &response, this->core_mask)) {
            LOG_ERROR("hashmap create");
        }
        if (STATUS(response->meta) != OK) {
            LOG_ERROR("hashmap create");
            return FAILED;
        }
        return OK;
    }

    int HashMapEngine::create(uint8_t flags, void (*cb)(void *)) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0, 0, "\0"};
        SET_OP_ID(payload.meta, CREATE_DS);
        SET_TYPE_ID(payload.meta, this->type_id);
        SET_DS_ID(payload.meta, this->ds_id);
        if (client->sendmsg_async(&payload, this->core_mask, cb) != 0) {
            LOG_ERROR("hashmap create async call failed");
        }
        return OK;
    }

    /* remove the data-structure altogether */
    int HashMapEngine::close() {
        pm_rpc_t *response;
        pm_rpc_t payload = {0, 0, "\0"};
        SET_OP_ID(payload.meta, CLOSE_DS);
        SET_TYPE_ID(payload.meta, this->type_id);
        SET_DS_ID(payload.meta, this->ds_id);
        if (!client->sendmsg(&payload, &response, this->core_mask)) {
            LOG_ERROR("hashmap close");
        }
        if (STATUS(response->meta) != OK) {
            LOG_ERROR("hashmap close");
            return FAILED;
        }
        return OK;
    }

    int HashMapEngine::close(void (*cb)(void *)) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0, 0, "\0"};
        SET_OP_ID(payload.meta, CLOSE_DS);
        SET_TYPE_ID(payload.meta, this->type_id);
        SET_DS_ID(payload.meta, this->ds_id);
        if (client->sendmsg_async(&payload, this->core_mask, cb) != 0) {
            LOG_ERROR("hashmap async close");
        }
    }



    /* remove the data-structure altogether */
    int HashMapEngine::remove() {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,REMOVE_DS);
        SET_TYPE_ID(payload.meta,this->type_id);
        SET_DS_ID(payload.meta,this->ds_id);
        if(!client->sendmsg(&payload,&response,this->core_mask)){
            LOG_ERROR("hashmap remove");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("hashmap remove");
            return FAILED;
        }
        return OK;
    }

    int HashMapEngine::remove(void (*cb)(void *)) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,REMOVE_DS);
        SET_TYPE_ID(payload.meta,this->type_id);
        SET_DS_ID(payload.meta,this->ds_id);
        if (client->sendmsg_async(&payload, this->core_mask, cb) != 0) {
            LOG_ERROR("hashmap async remove");
        }
    }


    std::string HashMapEngine::get(const unsigned long key) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_DS_ID(payload.meta, this->ds_id);
        SET_OP_ID(payload.meta,GET);
        SET_TYPE_ID(payload.meta,this->type_id);
        payload.key = key;
        if(!client->sendmsg(&payload,&response,this->core_mask)){
            LOG_ERROR("get operation");
        }
        if(STATUS(response->meta) != OK){
            return nullptr;
        }
        std::string ret_string(response->value);
        return ret_string;
    }

    int HashMapEngine::get(const unsigned long key,void (*cb)(void *)) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_DS_ID(payload.meta, this->ds_id);
        SET_OP_ID(payload.meta,GET);
        SET_TYPE_ID(payload.meta,this->type_id);
        payload.key = key;
        if (client->sendmsg_async(&payload, this->core_mask, cb) != 0) {
            LOG_ERROR("hashmap async get");
        }

    }


    int HashMapEngine::put(const unsigned long key, const std::string& value) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_DS_ID(payload.meta, this->ds_id);
        SET_OP_ID(payload.meta,PUT);
        SET_TYPE_ID(payload.meta,this->type_id);
        payload.key = key;
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",value.c_str());
        if(!client->sendmsg(&payload,&response,this->core_mask)){
            LOG_ERROR("hashmap put");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("hashmap put");
            return FAILED;
        }
        return OK;
    }

    int HashMapEngine::put(const unsigned long key, const std::string& value,void (*cb)(void *)) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_DS_ID(payload.meta, this->ds_id);
        SET_OP_ID(payload.meta,PUT);
        SET_TYPE_ID(payload.meta,this->type_id);
        payload.key = key;
        snprintf(payload.value,MAX_VAL_LENGTH,"%s",value.c_str());
        if (client->sendmsg_async(&payload, this->core_mask, cb) != 0) {
            LOG_ERROR("hashmap async put");
        }
        return OK;
    }


    int HashMapEngine::remove(const unsigned long key ) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_DS_ID(payload.meta, this->ds_id);
        SET_OP_ID(payload.meta,DELETE);
        SET_TYPE_ID(payload.meta,this->type_id);
        payload.key = key;
        if(!client->sendmsg(&payload,&response,this->core_mask)){
            LOG_ERROR("hashmap key delete");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("hashmap key delete");
            return FAILED;
        }
        return OK;
    }

    int HashMapEngine::remove(const unsigned long key ,void (*cb)(void *)) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_DS_ID(payload.meta, this->ds_id);
        SET_OP_ID(payload.meta,DELETE);
        SET_TYPE_ID(payload.meta,this->type_id);
        payload.key = key;
        if (client->sendmsg_async(&payload, this->core_mask, cb) != 0) {
            LOG_ERROR("hashmap async remove");
        }
        return OK;
    }

}
