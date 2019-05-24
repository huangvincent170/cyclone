/*
 * Pradeep Fernando, GT-ID : 903070752
 * pradeepfn@gatech.edu
 *
*/#ifndef PMEMDS_PRIORITY_QUEUE_H
#define PMEMDS_PRIORITY_QUEUE_H


#include "persistent_priority_queue.hpp"


namespace pmemds {

    class priority_queue : public PMEngine {
    public:

        priority_queue();

        ~priority_queue();


        void exec(uint16_t op_name,std::string& in_key, std::string& in_val, std::string& out_val,pm_rpc_t *resp);


        void insert(unsigned long key, unsigned long priority,pm_rpc_t *resp);

        void erase(unsigned long key,pm_rpc_t *resp);

        void increase_prio(const unsigned &key, unsigned long &delta_prio,pm_rpc_t *resp);

        void decrease_prio(const unsigned &key, unsigned long &delta_prio,pm_rpc_t *resp);


    private:
        persistent_priority_queue *pq_ptr;
    };

}


#endif //PMEMDS_PRIORITY_QUEUE_H
