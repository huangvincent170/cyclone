#include <climits>
#include "priority_queue.h"

namespace pmemds{


    void priority_queue::exec(uint16_t op_name,
                              uint8_t ds_type, std::string ds_id, unsigned long in_key, std::string& in_val, pm_rpc_t *resp) {

    }

    void priority_queue::insert(unsigned long key, unsigned long priority,pm_rpc_t *resp) {
        int ret = pq_ptr->insert(key,priority);
        if (ret == 0){
            SET_STATUS(resp->meta,OK);
            resp->key = key;
        }
        SET_STATUS(resp->meta, FAILED);
        resp->key = key;
    }


    void priority_queue::increase_prio(const unsigned &key, unsigned long &delta_prio,pm_rpc_t *resp) {
        int ret = pq_ptr->increase_prio(key,delta_prio);
        if(ret == 0){
            SET_STATUS(resp->meta,OK);
            resp->key = key;
        }
        SET_STATUS(resp->meta, FAILED);
        resp->key = key;
    }


}