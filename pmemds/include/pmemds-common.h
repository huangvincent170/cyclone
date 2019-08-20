#ifndef __PMEMDS_COMMON_H
#define __PMEMDS_COMMON_H


#define KEY_SIZE 8
#define MAX_VAL_LENGTH 32

#include "stdint.h"

// update, read operation flags
const uint8_t UPDATE_OPERATION = 0;
const uint8_t READ_OPERATION   = 1;

/// partition constatns
#define MAX_PARTITIONS 4


//common operations
enum opnames{
    /*common*/
    OPEN=0,
    CLOSE,
    CREATE_DS,
    CLOSE_DS,
    REMOVE_DS,
    /*tree*/
    GET,
    PUT,
    DELETE,
    /* priority queue specific */
    INSERT,
    INCREASE_PRIO,
    DECREASE_PRIO,
    GET_TOPK
};

//supported data-structures
enum dstypes{
    SORTED_BTREE = 0,
    CONCURRENT_HASHMAP,
    SHARDED_HASHMAP,
    SHARDED_PRIORITY_QUEUE,
    VECTOR,
};


typedef enum {                                             // status enumeration
    FAILED = UINT8_MAX,                                           // operation failed

    OK = 0 ,                     // successful completion
    NOT_FOUND = 1,
    INVALID_OP = 2
} PMStatus;

// metadata composition
// reserved(8)|STATUS(4)|DS_ID(8)|TYPE_ID(4)|OP_ID(8) -- 4 bytes

//request meta masks
#define OP_ID(x)      x & 0xff
#define TYPE_ID(x)    (x >> 8) & 0xf
#define DS_ID(x)      (x >> 12) & 0xff

#define SET_OP_ID(x,y)    x = x | y
#define SET_TYPE_ID(x,y)  x = x | (y << 8)
#define SET_DS_ID(x,y)    x = x | (y << 12)

/* ds client flags */
const uint8_t PM_CREAT = 1;


//response meta masks
#define STATUS(x)       ((x >> 20) & 0xf)
#define SET_STATUS(x,y)  x =  (x | (y << 20))


/* payload structure of pmemds lib */
typedef struct pm_rpc_st{
    unsigned int meta;
    union {
        unsigned long key;
        char ckey[8];
    };
    char value[MAX_VAL_LENGTH];
}pm_rpc_t;


#endif
