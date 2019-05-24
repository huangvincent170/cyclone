/*
 * Copyright 2017-2018, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <regex>
#include <unistd.h>

#include <libpmemobj++/transaction.hpp>
#include <libpmemobj++/make_persistent_atomic.hpp>

#include "btree.h"

#define DO_LOG 0
#define LOG(msg) if (DO_LOG) std::cout << "[btree] " << msg << "\n"

using pmem::obj::make_persistent_atomic;
using pmem::obj::transaction;
using pmem::detail::conditional_add_to_tx;

namespace pmemds {

BTreeEngine::BTreeEngine(const string& path, const size_t size) {
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

BTreeEngine::~BTreeEngine() {
    LOG("Closing");
    pmpool.close();
    LOG("Closed ok");
}

    void BTreeEngine::exec(uint16_t op_name, std::string &in_key, std::string &in_val, pm_rpc_t *resp) {

        switch (op_name){
            case GET:
                this->get(in_key,resp);
            case PUT:
                this->put(in_key,in_val,resp);
            default:
                LOG_ERROR("unknown operation");
                SET_STATUS(resp->meta,INVALID_OP);
        }
    }

void BTreeEngine::Exists(const string& key,pm_rpc_t *resp) {
    LOG("Exists for key=" << key);
    btree_type::iterator it = my_btree->find(pstring<20>(key));
    if (it == my_btree->end()) {
        LOG("  key not found");
        SET_STATUS(resp->meta,NOT_FOUND);
    }
    SET_STATUS(resp->meta,OK);
}

void BTreeEngine::get(void* context, const string& key, pm_rpc_t *resp) {
    LOG("Get using callback for key=" << key);
    btree_type::iterator it = my_btree->find(pstring<20>(key));
    if (it == my_btree->end()) {
        LOG("  key not found");
        SET_STATUS(resp->meta,NOT_FOUND);
    }
    SET_STATUS(resp->meta,OK);
    snprintf(resp->value,it->second.size(),it->second.c_str());
}

void BTreeEngine::put(const string& key, const string& value,pm_rpc_t *resp) {
    LOG("Put key=" << key << ", value.size=" << to_string(value.size()));
    auto res = my_btree->insert(std::make_pair(pstring<MAX_KEY_SIZE>(key), pstring<MAX_VALUE_SIZE>(value)));
    if (!res.second) { // key already exists, so update
        typename btree_type::value_type& entry = *res.first;
        transaction::manual tx(pmpool);
        conditional_add_to_tx(&(entry.second));
        entry.second = value;
        transaction::commit();
    }
    SET_STATUS(resp->meta,OK);
}

void BTreeEngine::remove(const string& key,pm_rpc_t *resp) {
    LOG("Remove key=" << key);
    size_t result = my_btree->erase(key);
    if (result == 1) {
        SET_STATUS(resp->meta,OK);
    }
    SET_STATUS(resp->meta,NOT_FOUND);
}

void BTreeEngine::Recover() {
    auto root_data = pmpool.get_root();
    if (root_data->btree_ptr) {
        my_btree = root_data->btree_ptr.get();
        my_btree->garbage_collection();
    } else {
        make_persistent_atomic<btree_type>(pmpool, root_data->btree_ptr);
        my_btree = root_data->btree_ptr.get();
    }
}

} // namespace pmemkv
