#ifndef PMEMDS_PRIORITY_QUEUE_CLIENT_H
#define PMEMDS_PRIORITY_QUEUE_CLIENT_H


#include "../pmemds-client.h"

namespace pmemdsclient{

    class priority_queue:public PMEngine{


    public:
        priority_queue(PMClient *handle, const std::string &path, size_t size, unsigned long core_mask);
        ~priority_queue();

        int create(uint8_t flags);
        int remove();

        int insert(unsigned long key, unsigned long priority);

        int erase(unsigned long key);

        int increase_prio(const unsigned &key, unsigned long &delta_prio);

        int decrease_prio(const unsigned &key, unsigned long &delta_prio);

    private:
        PMClient *client;
        std::string ds_name;
        std::string path;
        size_t size;
        unsigned long core_mask;
    };


}


#endif //PMEMDS_PRIORITY_QUEUE_CLIENT_H
