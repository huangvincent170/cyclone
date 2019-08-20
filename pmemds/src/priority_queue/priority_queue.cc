#include <climits>
#include "priority_queue/priority_queue.h"

#define DO_LOG 0
#define LOG(msg) if (DO_LOG) std::cout << "[btree] " << msg << "\n"

namespace pmemds{


    priority_queue::priority_queue(const string &path, size_t size, uint8_t npartitions){
        if(npartitions < 1) {
            npartitions = 1;
            LOG_INFO("npartitions defaulted to 1");
        }
        if(npartitions > MAX_PARTITIONS){
            LOG_ERROR("npartitions, should be less than MAX_PARTITIONS" + );
        }
        for(int i = 0; i < npartitions; i++) {
            this->my_pq[i] = new persistent_priority_queue();
        }
    }


    priority_queue::~priority_queue(){
    }

    void priority_queue::exec(uint8_t thread_id, uint16_t op_id, uint8_t ds_type, std::string ds_id,
                              pm_rpc_t *req, pm_rpc_t *resp){
         unsigned long priority;
         switch (op_id){
             case INSERT:
                 priority = parse_priority(req->value);
                 this->insert(thread_id, req->key,priority,resp);
                 break;
             case INCREASE_PRIO:
                 priority = parse_priority(req->value);
                 this->increase_prio(thread_id, req->key,priority,resp);
                 break;
             case DECREASE_PRIO:
                 priority = parse_priority(req->value);
                 this->decrease_prio(thread_id, req->key,priority,resp);
                 break;
             case GET_TOPK:
                 this->read_topk(thread_id, resp);
                 break;
             default:
                 LOG_ERROR("unknown operation");
                 SET_STATUS(resp->meta,INVALID_OP);
         }
    }


    void priority_queue::increase_prio(uint8_t thread_id, const unsigned &key, unsigned long &delta_prio,pm_rpc_t *resp) {
        LOG("increase priority = " << key);
        int ret = my_pq[thread_id]->increase_prio(key,delta_prio);
        resp->key = key;

        if(ret == 0){
            SET_STATUS(resp->meta,OK);
            return;
        }
        SET_STATUS(resp->meta, FAILED);
    }

    void priority_queue::decrease_prio(uint8_t thread_id,const unsigned &key, unsigned long &delta_prio, pm_rpc_t *resp) {
        LOG("decrease priority = " << key);
        resp->key = key;
        int ret = my_pq[thread_id]->decrease_prio(key,delta_prio);
        if(ret==0){
            SET_STATUS(resp->meta,OK);
            resp->key = key;
            return;
        }
        SET_STATUS(resp->meta, FAILED);

    }


    void priority_queue::insert(uint8_t thread_id, unsigned long key, unsigned long priority, pm_rpc_t *resp) {
        LOG("insert key = " << key);
        resp->key = key;
        int ret = my_pq[thread_id]->insert(key,priority);
        if(ret==0){
            SET_STATUS(resp->meta,OK);
            resp->key = key;
            return;
        }
        SET_STATUS(resp->meta, FAILED);
    }

    void priority_queue::read_topk(uint8_t thread_id, pm_rpc_t *resp) {
        LOG("get max");
        unsigned long key;
        unsigned long prio;
        int ret = my_pq[thread_id]->read_topK();
        if(ret==0){
            SET_STATUS(resp->meta,OK);
            resp->key = key;
            return;
        }
        SET_STATUS(resp->meta, FAILED);
    }

    void priority_queue::erase(unsigned long key, pm_rpc_t *resp) {

    }

    unsigned long priority_queue::parse_priority(char *data){
            unsigned long *prio = reinterpret_cast<unsigned long *>(data);
        return *prio;
    }
}