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

#pragma once

#include "../pmemds.h"
#include "btree/persistent_b_tree.h"
#include "btree/pstring.h"

using pmem::obj::pool;
using pmem::obj::persistent_ptr;

namespace pmemds {

                           // engine identifier
const size_t DEGREE = 64;
const size_t MAX_KEY_SIZE = 20;
const size_t MAX_VALUE_SIZE = 200;

class BTreeEngine : public PMEngine {
  public:
    BTreeEngine(const string& path, size_t size);          // default constructor
    ~BTreeEngine();                                        // default destructor

    void exec(uint8_t thread_id, uint16_t op_name, uint8_t ds_type, std::string ds_id,
                pm_rpc_t *req, pm_rpc_t *resp);

    string engine() final { return ENGINE; };
    const string ENGINE = "btree";
    void Exists(const unsigned long key ,pm_rpc_t *resp);              // does key have a value?

    void get(const unsigned long key, pm_rpc_t *resp);
    void put(const unsigned long key,                        // store key and value
                 const string& value,pm_rpc_t *resp);
    void remove(const unsigned long key,pm_rpc_t *resp);              // remove value for key
  private:
    BTreeEngine(const BTreeEngine&);
    void operator=(const BTreeEngine&);
    typedef persistent::b_tree<pstring<MAX_KEY_SIZE>,
            pstring<MAX_VALUE_SIZE>, DEGREE> btree_type;
    struct RootData {
        persistent_ptr<btree_type> btree_ptr;
    };
    void Recover();                                        // reload state from persistent pool
    pool<RootData> pmpool;                                 // pool for persistent root
    btree_type* my_btree;
};

} // namespace pmemkv
