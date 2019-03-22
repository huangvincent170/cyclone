#ifndef _BLIZ_MAP_
#define _BLIZ_MAP_

#include <unistd.h>

#define OP_PUT        0
#define OP_GET        1
#define OP_ADD        2

const unsigned long value_sz     = 8;
typedef struct blizop_st{
  unsigned long op;
  unsigned long key;
  char value[value_sz];
}blizop_t;


static inline int 
file_exists(char const *file)
{
    return access(file, F_OK);
}


#endif
