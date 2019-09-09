#ifndef PMEMDS_PRIORITY_QUEUE_H
#define PMEMDS_PRIORITY_QUEUE_H


#include "persistent_priority_queue.hpp"


namespace pmemds {

    class ShardedPriorityQueueEngine : public PMEngine {
    public:

        ShardedPriorityQueueEngine(const string& path, size_t size,uint8_t npartitions);
        ~ShardedPriorityQueueEngine();
        const string ENGINE = "prio_queue";
        void* engine(uint8_t thread_id){
            return my_pq[thread_id];
        };

        void exec(uint8_t thread_id, uint16_t op_id,uint8_t ds_type, std::string ds_id,
                  pm_rpc_t *req,pm_rpc_t **resp_ptr, int *resp_size);

        void insert(uint8_t thread_id, unsigned long key, unsigned long priority,pm_rpc_t *resp);

        void read_topk(uint8_t thread_id,pm_rpc_t *resp);

        void erase(unsigned long key,pm_rpc_t *resp);

        void increase_prio(uint8_t thread_id, const unsigned &key, unsigned long &delta_prio,pm_rpc_t *resp);

        void decrease_prio(uint8_t thread_id, const unsigned &key, unsigned long &delta_prio,pm_rpc_t *resp);


    private:
        unsigned long parse_priority(char *data);

        persistent_priority_queue *my_pq[MAX_PARTITIONS];
    };

}


#endif //PMEMDS_PRIORITY_QUEUE_H
