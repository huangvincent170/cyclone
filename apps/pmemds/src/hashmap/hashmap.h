
#pragma once

#include "../pmemds.h"
#include "pstring.h"
#include "concurrent_hash_map.hpp"

using pmem::obj::pool;
using pmem::obj::persistent_ptr;

namespace pmemds {

const string ENGINE = "hashmap";                             // engine identifier
const size_t MAX_KEY_SIZE = 20;
const size_t MAX_VALUE_SIZE = 200;

class HashMapEngine : public PMEngine {
  public:
    HashMapEngine(const string& path, size_t size);          // default constructor
    ~HashMapEngine();                                        // default destructor

    string Engine() final { return ENGINE; }               // engine identifier
    PMStatus Exists(const string& key) final;              // does key have a value?

    using PMEngine::Get;                                   // pass value to callback
    void Get(void* context,                                // pass value to callback with context
             const string& key,
             PMGetCallback* callback) final;

    PMStatus Put(const string& key,                        // store key and value
                 const string& value) final;
    PMStatus Remove(const string& key) final;              // remove value for key
  private:
    HashMapEngine(const HashMapEngine&);
    void operator=(const HashMapEngine&);
		typedef pmem::obj::experimental::concurrent_hash_map<pstring<MAX_KEY_SIZE>, pstring<MAX_VALUE_SIZE>> hashmap_type;
		struct RootData {
			        persistent_ptr<hashmap_type> btree_ptr;
							    }; 
    void Recover();                                        // reload state from persistent pool
    pool<RootData> pmpool;                                 // pool for persistent root
    hashmap_type* my_hashmap;
};

}
