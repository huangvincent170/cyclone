
#include "priority_queue-client.h"

namespace pmemdsclient {
    priority_queue::priority_queue(PMClient *handle,
                                   const std::string &path, size_t size, unsigned long core_mask) {
        this->core_mask = core_mask;
        this->path = path;
        this->size = size;
    }

    priority_queue::~priority_queue() {

    }

    int priority_queue::create(uint8_t flags) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0, 0, "\0"};
        SET_OP_ID(payload.meta, CREATE);
        SET_TYPE_ID(payload.meta, PRIORITY_QUEUE);
        snprintf(payload.value, MAX_VAL_LENGTH, "%s", ds_name.c_str());
        if (client->sendmsg(&payload, &response, this->core_mask) != 0) {
            LOG_ERROR("priority queue create");
        }
        if (response->meta != OK) {
            LOG_ERROR("priority queue insert");
            return STATUS(response->meta);
        }
        return OK;
    }

    int priority_queue::remove() {

    }



    int priority_queue::insert(unsigned long key, unsigned long priority) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,INSERT);
        payload.key = key;

        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("priority queue insert");
        }
        if(response->meta != OK){
            LOG_ERROR("priority queue insert");
            return STATUS(response->meta);
        }
        return OK;

    }

    int priority_queue::increase_prio(const unsigned &key, unsigned long &delta_prio) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,INSERT);
        payload.key = key;

        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("priority queue insert");
        }
        if(response->meta != OK){
            LOG_ERROR("priority queue insert");
            return STATUS(response->meta);
        }
        return OK;
    }

    int priority_queue::decrease_prio(const unsigned &key, unsigned long &delta_prio) {
        pm_rpc_t *response;
        pm_rpc_t payload = {0,0,"\0"};
        SET_OP_ID(payload.meta,INSERT);
        payload.key = key;

        if(client->sendmsg(&payload,&response,this->core_mask) != 0){
            LOG_ERROR("priority queue insert");
        }
        if(response->meta != OK){
            LOG_ERROR("priority queue insert");
            return STATUS(response->meta);
        }
        return OK;
    }


}