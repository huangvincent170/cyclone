
#pragma once

#include "../pmemds.h"
#include "../polymorphic_string.h"
#define LIBPMEMOBJ_CPP_USE_TBB_RW_MUTEX 1
#include "libpmemobj++/experimental/concurrent_hash_map.hpp"

using pmem::obj::pool;
using pmem::obj::persistent_ptr;

namespace pmemds {


    class key_equal {
    public:
        template <typename M, typename U>
        bool operator()(const M &lhs, const U &rhs) const
        {
            return lhs == rhs;
        }
    };


class string_hasher {
    /* hash multiplier used by fibonacci hashing */
    static const size_t hash_multiplier = 11400714819323198485ULL;

public:
    using transparent_key_equal = key_equal;

    size_t operator()(const pmemds::polymorphic_string &str) const
    {
        return hash(str.c_str(), str.size());
    }

    size_t operator()(string_view str) const
    {
        return hash(str.data(), str.size());
    }

private:
    size_t hash(const char *str, size_t size) const
    {
        size_t h = 0;
        for (size_t i = 0; i < size; ++i) {
            h = static_cast<size_t>(str[i]) ^ (h * hash_multiplier);
        }
        return h;
    }
};


    class HashMapEngine : public PMEngine {
    public:
        HashMapEngine(const string &path, size_t size);          // default constructor
        ~HashMapEngine();                                        // default destructor
        const string ENGINE = "hashmap";

        string engine() final { return ENGINE; }               // engine identifier

        void exec(uint16_t op_name,
                  uint8_t ds_type, std::string ds_id, pm_rpc_t *req, pm_rpc_t *resp);

        void exists(string_view key, pm_rpc_t *resp);              // does key have a value?

        void get(string_view key, pm_rpc_t *resp);

        void put(string_view key, string_view value, pm_rpc_t *resp);

        void remove(string_view key, pm_rpc_t *resp);              // remove value for key
    private:
        using string_t = pmemds::polymorphic_string;
        using map_t = pmem::obj::experimental::concurrent_hash_map<string_t, string_t, string_hasher>;

        struct RootData {
            pmem::obj::persistent_ptr<map_t> map_ptr;
        };
        using pool_t = pmem::obj::pool<RootData>;

        void Recover();
        pool_t pmpool;
        map_t *container;
    };



    /*
     * Partitioned hashmap engine.
     */
    class ShardedHashMapEngine : public PMEngine {
    public:
        ShardedHashMapEngine(uint8_t npartitions, const string &path, size_t size);          // default constructor
        ~ShardedHashMapEngine();                                        // default destructor
        const string ENGINE = "shardedhashmap";

        string engine() final { return ENGINE; }               // engine identifier

        void exec(uint8_t thread_id, uint16_t op_name,
                  uint8_t ds_type, std::string ds_id, pm_rpc_t *req, pm_rpc_t *resp);

        void exists(uint8_t thread_id, string_view key, pm_rpc_t *resp);              // does key have a value?

        void get(uint8_t thread_id, string_view key, pm_rpc_t *resp);

        void put(uint8_t thread_id, string_view key, string_view value, pm_rpc_t *resp);

        void remove(uint8_t thread_id, string_view key, pm_rpc_t *resp);              // remove value for key
    private:
        using string_t = pmemds::polymorphic_string;
        using map_t = pmem::obj::experimental::concurrent_hash_map<string_t, string_t, string_hasher>;

        struct RootData {
            pmem::obj::persistent_ptr<map_t> map_ptr;
        };
        using pool_t = pmem::obj::pool<RootData>;

        void Recover(uint8_t npartitions);
        pool_t pmpool;
        map_t *container;
    };

}
