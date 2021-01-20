#ifndef _ROCKSDB_COMMON_
#define _ROCKSDB_COMMON_

const unsigned long OP_ADD_EDGE       = 0;
const unsigned long OP_ADD_VERTEX     = 1;
const unsigned long OP_DEL_EDGE       = 3;
const unsigned long OP_DEL_VERTEX     = 4;
const unsigned long OP_OUTDEGREE      = 5;


typedef struct llama_req_st{
  unsigned long op;
  unsigned long data1;
  unsigned long data2;
}llama_req_t;

typedef struct llama_res_st{
	unsigned long state;
	unsigned long outdegree;
}llama_res_t;

const unsigned long max_llama_nodes = 1000000;
const char *llama_db = "/mnt/pmem0/llama_db";
#endif
