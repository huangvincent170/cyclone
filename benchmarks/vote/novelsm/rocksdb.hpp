#ifndef _ROCKSDB_COMMON_
#define _ROCKSDB_COMMON_
const unsigned long OP_PUT       = 0;
const unsigned long OP_GET       = 1;
const unsigned long OP_INCR      = 2;
const unsigned long OP_DECR      = 3;

const unsigned long value_sz     = 16;

/// article and vote key's prefix
const unsigned long ART		     = 44;
const unsigned long VOTE		 = 55;

/**  need atleast one char qualifier to represent vote app in rocksdb
     original norria benchmark uses vote_<articleId> and vote_vc<articleId> **/  
typedef struct votekey_st{ 
	unsigned long  prefix;
	unsigned long art_id;
} votekey_t; /* with data alignment even if i use smaller data type it takes 16 bytes of space */

typedef struct rockskv_st{
  unsigned long op;
  votekey_t key;
  union{
	char value[value_sz];
	unsigned long prio;
  };
}rockskv_t;


const char *preload_dir = "/mnt/pmem0/votepreloaded";
const char *data_dir = "/mnt/pmem0/voterocksdata";
const char *log_dir  = "/mnt/pmem0/voterockswal";
const unsigned long rocks_keys = 1000000;
const int use_rocksdbwal = 1;
#endif
