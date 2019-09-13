#ifndef __PMEMDS_COMMON_H
#define __PMEMDS_COMMON_H


#define KEY_SIZE 8
#define MAX_VAL_LENGTH 16

#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"

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
    GET_TOPK,

    /* adjvector/graph structure specific */
    ADD_EDGE,
    REMOVE_EDGE,
    ADD_NODE,
    REMOVE_NODE,
    VERTEX_OUTDEGREE,
    INCIDENT_TRIANGLES
};

//supported data-structures
enum dstypes{
    SORTED_BTREE = 0,
    CONCURRENT_HASHMAP,
    SHARDED_HASHMAP,
    SHARDED_PRIORITY_QUEUE,
    ADJACENCY_VECTOR,
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
    union {
        char value[MAX_VAL_LENGTH];
        unsigned long key2;
    };
}pm_rpc_t;


#define ART_ARRAY_SIZE 16

static const unsigned long MAX_VOTE_PAYLOAD = 100;
/// vote benchmark specific payload
struct vote_payload_st{
    unsigned long idx;
    char art[ART_ARRAY_SIZE];       // article
};


static inline void* safe_malloc(size_t n, unsigned long line)
{
    void* p = malloc(n);
    if (!p)
    {
        fprintf(stderr, "[%s:%lu]Out of memory\n",
                __FILE__, line);
        exit(EXIT_FAILURE);
    }
    return p;
}

static inline void* safe_calloc(size_t n, unsigned long line)
{
    void* p = calloc(1,n);
    if (!p)
    {
        fprintf(stderr, "[%s:%lu]Out of memory\n",
                __FILE__, line);
        exit(EXIT_FAILURE);
    }
    return p;
}

//#define SAFEMALLOC(n) safe_malloc(n, __LINE__)
#define SAFECALLOC(n) safe_calloc(n, __LINE__)

#endif
