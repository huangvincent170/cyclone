#ifndef _ROCKSDB_COMMON_
#define _ROCKSDB_COMMON_
const unsigned long OP_PUT       = 0;
const unsigned long OP_GET       = 1;
const unsigned long OP_INCR      = 2;
const unsigned long OP_DECR      = 3;

const unsigned long value_sz     = 16;


/**  need atleast one char qualifier to represent vote app in rocksdb
     original norria benchmark uses vote_<articleId> and vote_vc<articleId> **/  
typedef struct votekey_st{
	char prefix;
	unsigned long art_id;
} votekey_t;

typedef struct rockskv_st{
  unsigned long op;
  votekey_t key;
  union{
	char value[value_sz];
	unsigned long prio;
  };
}rockskv_t;


const char *preload_dir = "/mnt/pmem1/preloaded";
const char *data_dir = "/mnt/pmem1/rocksdata";
const char *log_dir  = "/mnt/pmem1/rockswal";
const unsigned long rocks_keys = 1000000;
const int use_rocksdbwal = 1;
#endif
