
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
    HashMapEngine(const string& path, size_t size);          // default constructor
    ~HashMapEngine();                                        // default destructor
		const string ENGINE = "hashmap";
		string Engine() final { return ENGINE; }               // engine identifier
    PMStatus Exists(const string& key) final;              // does key have a value?

    using PMEngine::get;                                   // pass value to callback
    void get(void* context,                                // pass value to callback with context
             const string& key,
             PMGetCallback* callback) final;

    PMStatus put(const string& key,                        // store key and value
                 const string& value) final;
    PMStatus remove(const string& key) final;              // remove value for key
  private:
    HashMapEngine(const HashMapEngine&);
    void operator=(const HashMapEngine&);
		typedef pmem::obj::experimental::concurrent_hash_map<pstring<MAX_KEY_SIZE>, pstring<MAX_VALUE_SIZE>> hashmap_type;
		struct RootData {
			        persistent_ptr<hashmap_type> hashmap_ptr;
							    }; 
    void Recover();                                        // reload state from persistent pool
    pool<RootData> pmpool;                                 // pool for persistent root
    hashmap_type* my_hashmap;
};

}
