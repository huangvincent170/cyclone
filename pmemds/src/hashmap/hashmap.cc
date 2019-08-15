
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
                             uint8_t ds_type, std::string ds_id, pm_rpc_t *req, pm_rpc_t *resp) {
        string_view in_key(req->ckey, KEY_SIZE);

        switch (op_name){
            case GET:
				//LOG("Get op : " << in_key.);
                this->get(in_key,resp);
                break;
            case PUT: {
                std::string val = std::string(req->value);
                string_view in_val(val);
                //LOG("Put op : " << in_key << in_val);
                this->put(in_key, in_val, resp);
            }
                break;
            default:
                LOG_ERROR("unknown operation");
                SET_STATUS(resp->meta,INVALID_OP);
        }

    }


    void HashMapEngine::exists(string_view key, pm_rpc_t *resp) {
        //LOG("exists for key=" << key);
        container->count(key) == 1 ? SET_STATUS(resp->meta,OK) : SET_STATUS(resp->meta, NOT_FOUND);
    }


    void HashMapEngine::get(string_view key, pm_rpc_t *resp) {

        //LOG("get key= " << key);
        map_t ::const_accessor result;
        bool found = container->find(result, key);
        if (!found) {
            LOG("  key not found");
            SET_STATUS(resp->meta, NOT_FOUND);
            return;
        }
        SET_STATUS(resp->meta,OK);
        snprintf(resp->value,MAX_VAL_LENGTH, "%s",result->second.c_str());

    }

    void HashMapEngine::put(string_view key, string_view value, pm_rpc_t *resp) {

        //LOG("Put key=" << key << ", value.size=" << to_string(value.size()));

        map_t ::accessor acc;
        // XXX - do not create temporary string
        bool result =
                container->insert(acc, map_t::value_type(string_t(key), string_t(value)));
        if (!result) {
            pmem::obj::transaction::manual tx(pmpool);
            acc->second = value;
            pmem::obj::transaction::commit();
        }
        SET_STATUS(resp->meta,OK);

    }

    void HashMapEngine::remove(string_view key, pm_rpc_t *resp) {
        //LOG("remove key=" << key);

        bool erased = container->erase(key);
        erased ? SET_STATUS(resp->meta,OK) : SET_STATUS(resp->meta, NOT_FOUND);
    }

    void HashMapEngine::Recover() {
        auto root_data = pmpool.root();
        if (root_data->map_ptr) {
            container = root_data->map_ptr.get();
            container->initialize();
        } else {
            pmem::obj::transaction::manual tx(pmpool);
            root_data->map_ptr = pmem::obj::make_persistent<map_t>();
            pmem::obj::transaction::commit();
            container = root_data->map_ptr.get();
            container->initialize(true);
        }

    }

} // namespace pmemds
