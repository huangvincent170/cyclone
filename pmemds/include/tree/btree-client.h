#ifndef PMEMDS_BTREE_CLIENT_H
#define PMEMDS_BTREE_CLIENT_H

#include "../pmemds-client.h"
#include "../pmemds_log.h"

namespace pmemdsclient{

    class BTreeEngine:public PMEngine{

    public:
        BTreeEngine(PMClient *handle, const uint16_t ds_id, size_t size, unsigned long core_mask);
        ~BTreeEngine();


        int create(uint8_t flags);
        int close();
        int remove();

        //sync member functions
        std::string get(const unsigned long key);
        int put(const unsigned long key, const std::string& value);
        int remove(const unsigned long key);

        //async member functions
        std::string get(const unsigned long key);
        int put(const unsigned long key, const std::string& value);
        int remove(const unsigned long key);
    private:
        std::string ds_name;
    };
}

#endif //PMEMDS_BTREE_CLIENT_H
