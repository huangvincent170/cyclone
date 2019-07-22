
#pragma once

#include "../pmemds.h"
#include "pstring.h"
#include "concurrent_hash_map.hpp"


#define MAX_KEY_SIZE 20
#define MAX_VALUE_SIZE 100

using pmem::obj::pool;
using pmem::obj::persistent_ptr;

namespace pmemds {


    class HashMapEngine : public PMEngine {
    public:
        HashMapEngine(const string &path, size_t size);          // default constructor
        ~HashMapEngine();                                        // default destructor
        const string ENGINE = "hashmap";

        string engine() final { return ENGINE; }               // engine identifier

        void exec(uint16_t op_name,
                  uint8_t ds_type, std::string ds_id, unsigned long in_key, pm_rpc_t *req, pm_rpc_t *resp);

        void Exists(const unsigned long key, pm_rpc_t *resp);              // does key have a value?

        void get(const unsigned long key, pm_rpc_t *resp);

        void put(const unsigned long key, const string &value, pm_rpc_t *resp);

        void remove(const unsigned long key, pm_rpc_t *resp);              // remove value for key
    private:
        HashMapEngine(const HashMapEngine &);

        void operator=(const HashMapEngine &);

        typedef pmem::obj::experimental::concurrent_hash_map<unsigned long, pstring<MAX_VALUE_SIZE>> hashmap_type;
        struct RootData {
            persistent_ptr<hashmap_type> hashmap_ptr;
        };

        void Recover();                                        // reload state from persistent pool
        pool<RootData> pmpool;                                 // pool for persistent root
        hashmap_type *my_hashmap;
        hashmap_type::accessor a;
    };

}
