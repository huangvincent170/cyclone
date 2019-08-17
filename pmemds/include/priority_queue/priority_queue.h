#ifndef PMEMDS_PRIORITY_QUEUE_H
#define PMEMDS_PRIORITY_QUEUE_H


#include "persistent_priority_queue.hpp"


namespace pmemds {

    class priority_queue : public PMEngine {
    public:

        priority_queue(const string& path, size_t size);
        ~priority_queue();
        const string ENGINE = "prio_queue";
        string engine() final { return ENGINE; };

        void exec(uint16_t op_id,uint8_t ds_type, std::string ds_id,
                  pm_rpc_t *req,pm_rpc_t *resp);

        void insert(unsigned long key, unsigned long priority,pm_rpc_t *resp);

        void get_max(pm_rpc_t *resp);

        void erase(unsigned long key,pm_rpc_t *resp);

        void increase_prio(const unsigned &key, unsigned long &delta_prio,pm_rpc_t *resp);

        void decrease_prio(const unsigned &key, unsigned long &delta_prio,pm_rpc_t *resp);


    private:
        unsigned long parse_priority(char *data);

        persistent_priority_queue *my_pq;
    };

}


#endif //PMEMDS_PRIORITY_QUEUE_H
