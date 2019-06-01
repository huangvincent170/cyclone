#include <climits>
#include "priority_queue/priority_queue.h"

namespace pmemds{


    priority_queue::priority_queue(const string &path, size_t size){

    }


    priority_queue::~priority_queue(){
    }

    void priority_queue::exec(uint16_t op_id, uint8_t ds_type, std::string ds_id, unsigned long in_key,
                              std::string &in_val, pm_rpc_t *resp){

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

    void priority_queue::decrease_prio(const unsigned &key, unsigned long &delta_prio, pm_rpc_t *resp) {

    }


    void priority_queue::insert(unsigned long key, unsigned long priority, pm_rpc_t *resp) {

    }

    void priority_queue::get_max(pm_rpc_t *resp) {


    }

    void priority_queue::erase(unsigned long key, pm_rpc_t *resp) {

    }
}