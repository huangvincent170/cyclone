#ifndef __PMEMDS_COMMON_H
#define __PMEMDS_COMMON_H


#define MAX_VAL_LENGTH 8

#include "stdint.h"

//common operations
enum opnames{
    OPEN=0,
    CLOSE,
    CREATE,
    DELETE,
    GET,
    PUT,
    /* priority queue specific */
    INSERT,
    INCREASE_PRIO,
    DECREASE_PRIO
};

//supported data-structures
enum dstypes{
    SORTED_BTREE = 0,
    HASH_MAP,
    PRIORITY_QUEUE,
    VECTOR
};


typedef enum {                                             // status enumeration
    FAILED = UINT8_MAX,                                           // operation failed
    NOT_FOUND = 0,                                         // key not located
    OK = 1 ,                                                // successful completion
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



//response meta masks
#define STATUS(x)       (x >> 20) & 0xf
#define SET_STATUS(x,y) x = x | (y << 20)


/* payload structure of pmemds lib */
typedef struct pm_rpc_st{
    unsigned int meta;
    unsigned long key;
    char value[MAX_VAL_LENGTH];
}pm_rpc_t;


#endif