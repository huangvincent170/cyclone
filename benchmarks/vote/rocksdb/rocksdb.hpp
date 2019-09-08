#ifndef _ROCKSDB_COMMON_
#define _ROCKSDB_COMMON_
const unsigned long OP_PUT       = 0;
const unsigned long OP_GET       = 1;
const unsigned long OP_INCR      = 2;
const unsigned long OP_DECR      = 3;

const unsigned long value_sz     = 16;


/**  need atleast one char qualifier to represent vote app in rocksdb
     original norria benchmark uses vote_<articleId> and vote_vc<articleId> **/  
typedef struct key_st{
	char prefix[1];
	unsigned long art_id;
} key_t;

typedef struct rockskv_st{
  unsigned long op;
  key_t key;
  char value[value_sz];
}rockskv_t;


const char *preload_dir = "/mnt/pmem1/preloaded";
const char *data_dir = "/mnt/pmem1/rocksdata";
const char *log_dir  = "/mnt/pmem1/rockswal";
const unsigned long rocks_keys = 1000000;
const int use_rocksdbwal = 1;
#endif
