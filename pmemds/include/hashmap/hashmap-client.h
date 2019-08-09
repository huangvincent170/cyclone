#ifndef PMEMDS_HASHMAP_CLIENT_H
#define PMEMDS_HASHMAP_CLIENT_H

#include "../pmemds-client.h"
#include "../pmemds_log.h"

namespace pmemdsclient{

    class HashMapEngine:public PMEngine{

    public:
        HashMapEngine(PMClient *handle, const uint16_t ds_id, size_t size, unsigned long core_mask);
        ~HashMapEngine();


        int create(uint8_t flags);
        int close();
        int remove();

        int create(uint8_t flags,void (*cb)(void *));
        int close(void (*cb)(void *));
        int remove(void (*cb)(void *));


        //sync member functions
        std::string get(const unsigned long key);
        int put(const unsigned long key, const std::string& value);
        int remove(const unsigned long key);

        //async member functions
        int get(const unsigned long key,void (*cb)(void *));
        int put(const unsigned long key, const std::string& value,void (*cb)(void *));
        int remove(const unsigned long key,void (*cb)(void *));
    private:
        std::string ds_name;
    };
}

#endif //PMEMDS_HASHMAP_CLIENT_H
