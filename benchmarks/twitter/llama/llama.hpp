#ifndef _ROCKSDB_COMMON_
#define _ROCKSDB_COMMON_

const unsigned long OP_ADD_EDGE       = 0;
const unsigned long OP_ADD_VERTEX     = 1;
const unsigned long OP_DEL_EDGE       = 3;
const unsigned long OP_DEL_VERTEX     = 4;

const unsigned long value_sz     = 8;

typedef struct llama_st{
  unsigned long op;
  unsigned long data1;
  unsigned long data2;
}llama_t;

const char *mmap_dir = "/mnt/pmem1/llamadata";
#endif
