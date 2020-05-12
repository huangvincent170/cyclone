#ifndef _CLWBOPTSIM_
#define _CLWBOPTSIM_

#include <stdint.h>
#include <x86intrin.h>

/*
static const int CLFLUSHOPT_WINDOW = 10;

static void clflush(void *ptr, int size)
{
  char *x = (char *)ptr;
  while(size > 64*CLFLUSHOPT_WINDOW) {
    asm volatile("clflush %0"::"m"(*x));
    x    += 64*CLFLUSHOPT_WINDOW;
    size -= 64*CLFLUSHOPT_WINDOW;
  }
  asm volatile("clflush %0"::"m"(*x));
}

static int clflush_partial(void *ptr, int size, int clflush_cnt)
{
  char *x = (char *)ptr;
  while(size > 64) {
    clflush_cnt++;
    if(clflush_cnt == CLFLUSHOPT_WINDOW) {
      asm volatile("clflush %0"::"m"(*x));
      clflush_cnt = 0;
    }
    x    += 64;
    size -= 64;
  }
  clflush_cnt++;
  if(clflush_cnt == CLFLUSHOPT_WINDOW) {
    asm volatile("clflush %0"::"m"(*x));
    clflush_cnt = 0;
  }
  return clflush_cnt;
}
*/

#define FLUSH_ALIGN ((uintptr_t)64)

static void
flush_clwb(const void *addr, size_t len)
{
    uintptr_t uptr;
    /*
     * Loop through cache-line-size (typically 64B) aligned chunks
     * covering the given range.
     */
    for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
         uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN) {
         _mm_clwb((char *)uptr);
    }
}

static void
flush_clflush(const void *addr, size_t len)
{

    uintptr_t uptr;

    /*
     * Loop through cache-line-size (typically 64B) aligned chunks
     * covering the given range.
     */
    for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
         uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN) {
        _mm_clflush((char *) uptr);
    }
}



static inline void asm_sfence(void)
{
    __asm__ __volatile__ ("sfence");
}



#endif
