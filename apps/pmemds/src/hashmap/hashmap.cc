
#include <iostream>
#include <regex>
#include <unistd.h>

#include <libpmemobj++/transaction.hpp>
#include <libpmemobj++/make_persistent_atomic.hpp>

#include "hashmap.h"

#define DO_LOG 0
#define LOG(msg) if (DO_LOG) std::cout << "[hashmap] " << msg << "\n"

using pmem::obj::make_persistent_atomic;
using pmem::obj::transaction;
using pmem::detail::conditional_add_to_tx;

namespace pmemds {

HashMapEngine::HashMapEngine(const string& path, const size_t size) {
    if ((access(path.c_str(), F_OK) != 0) && (size > 0)) {
        LOG("Creating filesystem pool, path=" << path << ", size=" << to_string(size));
        pmpool = pool<RootData>::create(path.c_str(), LAYOUT, size, S_IRWXU);
    } else {
        LOG("Opening pool, path=" << path);
        pmpool = pool<RootData>::open(path.c_str(), LAYOUT);
    }
    Recover();
    LOG("Opened ok");
}

HashMapEngine::~HashMapEngine() {
    LOG("Closing");
    pmpool.close();
    LOG("Closed ok");
}

PMStatus HashMapEngine::Exists(const string& key) {
     return OK;
}

void HashMapEngine::get(void* context, const string& key, PMGetCallback* callback) {
	  hashmap_type::accessor access;
		my_hashmap->find(access,pstring<20>(key));
		//access->release();
    LOG("Get using callback for key=" << key);
}

PMStatus HashMapEngine::put(const string& key, const string& value) {
    LOG("Put key=" << key << ", value.size=" << to_string(value.size()));
		hashmap_type::accessor access;
		my_hashmap->insert(access,pstring<20>(key));
		access->second = pstring<100>(value);
		//access->release();
    return OK;
}

PMStatus HashMapEngine::remove(const string& key) {
    LOG("Remove key=" << key);
		my_hashmap->erase(pstring<20>(key));
    return OK;
}

void HashMapEngine::Recover() {
	auto root_data = pmpool.get_root();
	make_persistent_atomic<hashmap_type>(pmpool, root_data->hashmap_ptr);
  my_hashmap = root_data->hashmap_ptr.get();
}

} // namespace pmemds