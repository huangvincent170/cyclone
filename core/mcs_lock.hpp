#ifndef _MCS_LOCK_H
#define _MCS_LOCK_H

#include <cstdint>
#include <assert.h>
#include <threads.h>

#define TRUE  1
#define FALSE 0

/* Compile read-write barrier */
#define barrier() asm volatile("": : :"memory")

inline static void cpu_relax() {  
    asm volatile("pause\n": : :"memory");
}



struct mcs_node_t {
	uint8_t locked;
    uint8_t pad1[CACHELINE_SIZE - sizeof(uint8_t)];
	mcs_node* next;
    uint8_t pad2[CACHELINE_SIZE - sizeof(mcs_node*)];
};

typedef struct mcs_node* mcs_lock; 
//static __thread struct mcs_node local_node = {TRUE,NULL};


    void spin_lock(mcs_lock *tail) {
		struct mcs_node *prior_node = __sync_val_compare_and_swap(tail,NULL,&local_node);
        if(prior_node != NULL) {
            local_node.locked = TRUE;
			local_node.next = NULL;
            prior_node->next = &local_node;
			barrier();
            while(local_node.locked)
                cpu_relax();
        }
    }

    void spin_unlock(mcs_lock *tail) {
        if(local_node.next == NULL) {
        //if so, then either:
        // 1. no other thread is contending for the lock
        // 2. there is a race condition with another thread about to acquire the lock
            mcs_node* p = &local_node;
			if(__sync_val_compare_and_swap(tail,p,NULL) == p) {
                return;
            }
            while (local_node.next == NULL) {};
        }
        local_node.next->locked = FALSE;
    }

#endif
