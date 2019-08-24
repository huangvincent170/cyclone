
#include "priority_queue/priority_queue-client.h"

namespace pmemdsclient {
    PriorityQueueEngine::PriorityQueueEngine(PMClient *handle,
                                   const uint16_t ds_id, size_t size, unsigned long core_mask) {
        this->client = handle;
        this->ds_id = ds_id;
        this->core_mask = core_mask;
        this->size = size;
    }

    PriorityQueueEngine::~PriorityQueueEngine() {

    }

    int PriorityQueueEngine::create(uint8_t flags) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0, 0, "\0"};
        SET_OP_ID(payload.meta, CREATE_DS);
        SET_TYPE_ID(payload.meta, SHARDED_PRIORITY_QUEUE);
        SET_DS_ID(payload.meta,this->ds_id);
        if (client->sendmsg(&payload, &response, this->core_mask) != 0) {
            LOG_ERROR("priority queue create");
        }
        if (STATUS(response->meta) != OK) {
            LOG_ERROR("priority queue insert");
            return FAILED;
        }
        return OK;
    }


    int PriorityQueueEngine::close(){
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,CLOSE_DS);
        SET_TYPE_ID(payload.meta,SHARDED_PRIORITY_QUEUE);
        SET_DS_ID(payload.meta,this->ds_id);
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("priority_queue close");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("PriorityQueueEngine close");
            return FAILED;
        }
        return OK;
    }

    int PriorityQueueEngine::remove(){
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,REMOVE_DS);
        SET_TYPE_ID(payload.meta,SHARDED_PRIORITY_QUEUE);
        SET_DS_ID(payload.meta,this->ds_id);
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("priority_queue remove");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("priority_queue remove");
            return FAILED;
        }
        return OK;
    }

    int PriorityQueueEngine::insert(unsigned long key, unsigned long priority) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,INSERT);
        SET_DS_ID(payload.meta,this->ds_id);
        payload.key = key;
        unsigned long *value = reinterpret_cast<unsigned long *>(payload.value);
        *value = priority;
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("priority queue insert");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("priority queue insert");
            return FAILED;
        }
        return OK;

    }

    int PriorityQueueEngine::topk() {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,GET_TOPK);
        SET_DS_ID(payload.meta,this->ds_id);
        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("priority queue getmax");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("priority queue getmax");
            return FAILED;
        }
        //max = response->key;
        return OK;
    }

    int PriorityQueueEngine::increase_prio(const unsigned &key, unsigned long &delta_prio) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,INSERT);
        SET_DS_ID(payload.meta,this->ds_id);
        payload.key = key;
        unsigned long *value = reinterpret_cast<unsigned long *>(response->value);
        *value = delta_prio;

        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("priority queue insert");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("priority queue insert");
            return FAILED;
        }
        return OK;
    }

    int PriorityQueueEngine::decrease_prio(const unsigned &key, unsigned long &delta_prio) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,INSERT);
        SET_DS_ID(payload.meta,this->ds_id);
        payload.key = key;
        unsigned long *value = reinterpret_cast<unsigned long *>(response->value);
        *value = delta_prio;

        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("priority queue insert");
        }
        if(STATUS(response->meta) != OK){
            LOG_ERROR("priority queue insert");
            return STATUS(response->meta);
        }
        return OK;
    }

}