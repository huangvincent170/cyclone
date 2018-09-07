/*
 * Copyright (c) 2015, Intel Corporation
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
 *     * Neither the name of Intel Corporation nor the names of its
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


#include<assert.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <time.h>
#include<unistd.h>

#include "../core/libcyclone.hpp"
#include "../core/logging.hpp"
#include "../core/clock.hpp"
#include "libpmemkv.h"
#include "pmemkv.hpp"

static void *logs[executor_threads];
pmemkv::KVEngine *kv_;



void callback(const unsigned char *data,
              const int len,
              rpc_cookie_t *cookie)
{
	cookie->ret_value = malloc(len);
	cookie->ret_size = len;
	pmemkv_t *pmemkv = (pmemkv_t *) data;
	KVStatus s;
	if(pmemkv->op == OP_PUT){
		s = kv_->Put(std::to_string(pmemkv->key),std::string(pmemkv->value));
		if(s != OK){
			BOOST_LOG_TRIVIAL(error) << "Out of space at key : " << std::to_string(pmemkv->key);
			exit(1);
		}	
	memcpy(cookie->ret_value, data , len);

	}else if(pmemkv->op == OP_GET){
		pmemkv_t *pmemkv_back = (pmemkv_t *) cookie->ret_value;
		std::string value;
		s = kv_->Get(std::to_string(pmemkv->key), &value);	
		if(s != OK){
			pmemkv_back->key = ULONG_MAX;
		}else{
			pmemkv_back->key = pmemkv->key;
			memcpy(pmemkv_back->value,value.c_str() , value_sz);
		}

	}else{
		BOOST_LOG_TRIVIAL(fatal) << "wrong key-op : "<< pmemkv->op
			<< "key : "<<pmemkv->key;
	}
}

int wal_callback(const unsigned char *data,
                 const int len,
                 rpc_cookie_t *cookie)
{
	return cookie->log_idx;
}


void gc(rpc_cookie_t *cookie)
{
	free(cookie->ret_value);
}

rpc_callbacks_t rpc_callbacks =
{
	callback,
	gc,
	wal_callback
};



void opendb()
{
	assert(kv_ == NULL);
	size_t db_size = (size_t)(1024 * 1024 * 1024 * DB_SIZE_IN_GB);
	kv_ = pmemkv::KVEngine::Open(KV_ENGINE, PMEM_KV, db_size);
	if (kv_ == nullptr)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Cannot open db "<< std::string(PMEM_KV)
			<< "with size "<< std::to_string(DB_SIZE_IN_GB);
		exit(-1);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 7)
	{
		printf("Usage1: %s replica_id replica_mc clients cluster_config quorum_config ports\n", argv[0]);
		exit(-1);
	}

	int server_id = atoi(argv[1]);
	cyclone_network_init(argv[4],
	                     atoi(argv[6]),
	                     atoi(argv[2]),
	                     atoi(argv[6]) + num_queues * num_quorums + executor_threads);

	opendb();

	dispatcher_start(argv[4],
	                 argv[5],
	                 &rpc_callbacks,
	                 server_id,
	                 atoi(argv[2]),
	                 atoi(argv[3]));

}

