
#include <iostream>
#include <regex>
#include <unistd.h>

#include "hashmap/hashmap.h"

#define DO_LOG 0
#define LOG(msg) if (DO_LOG) std::cout << "[hashmap] " << msg << "\n"

using pmem::obj::make_persistent_atomic;
using pmem::obj::transaction;
using pmem::detail::conditional_add_to_tx;

namespace pmemds {

    HashMapEngine::HashMapEngine(const string &path, const size_t size) {
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


    void HashMapEngine::exec(uint16_t op_name,
                             uint8_t ds_type, std::string ds_id, unsigned long in_key, pm_rpc_t *req, pm_rpc_t *resp) {
        std::string in_val;
        switch (op_name){
            case GET:
				LOG("Get op : " << in_key);
                this->get(in_key,resp);
                break;
            case PUT:
                in_val = std::string(req->value);
				LOG("Put op : " << in_key << in_val);
                this->put(in_key,in_val,resp);
                break;
            default:
                LOG_ERROR("unknown operation");
                SET_STATUS(resp->meta,INVALID_OP);
        }

    }


    void HashMapEngine::Exists(const unsigned long key, pm_rpc_t *resp) {
        LOG("exists for key=" << key);
        my_hashmap->count(key) == 1 ? SET_STATUS(resp->meta,OK) : SET_STATUS(resp->meta, NOT_FOUND);
    }


    void HashMapEngine::get(const unsigned long key, pm_rpc_t *resp) {
        LOG("get key= " << key);
        hashmap_type::const_accessor result;
        bool found = my_hashmap->find(result, key);
        if (!found) {
            LOG("  key not found");
            SET_STATUS(resp->meta, NOT_FOUND);
            return;
        }
        SET_STATUS(resp->meta,OK);
        snprintf(resp->value,MAX_VAL_LENGTH, "%s",result->second.c_str());

    }

    void HashMapEngine::put(const unsigned long key, const string &value, pm_rpc_t *resp) {
        LOG("Put key=" << key << ", value.size=" << to_string(value.size()));

        hashmap_type::accessor acc;
        // XXX - do not create temporary string
        bool result =
                my_hashmap->insert(acc, hashmap_type::value_type(key, pstring<MAX_VALUE_SIZE>(value)));
        if (!result) {
            pmem::obj::transaction::manual tx(pmpool);
            acc->second = value;
            pmem::obj::transaction::commit();
        }
        SET_STATUS(resp->meta,OK);
    }

    void HashMapEngine::remove(const unsigned long key, pm_rpc_t *resp) {
        LOG("remove key=" << key);

        bool erased = my_hashmap->erase(key);
        erased ? SET_STATUS(resp->meta,OK) : SET_STATUS(resp->meta, NOT_FOUND);
    }

    void HashMapEngine::Recover() {
        auto root_data = pmpool.root();
        if (root_data->hashmap_ptr) {
            my_hashmap = root_data->hashmap_ptr.get();
            my_hashmap->initialize();
        } else {
            pmem::obj::transaction::manual tx(pmpool);
            root_data->hashmap_ptr = pmem::obj::make_persistent<hashmap_type>();
            pmem::obj::transaction::commit();
            my_hashmap = root_data->hashmap_ptr.get();
            my_hashmap->initialize(true);
        }
    }

} // namespace pmemds
