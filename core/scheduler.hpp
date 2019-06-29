/*
 * Blizzard schduler impl. 
 *
 */
#ifndef CYCLONE_SCHEDULER_QUEUE_HPP
#define CYCLONE_SCHEDULER_QUEUE_HPP

#include <rte_mbuf.h>
#include <rte_malloc.h>

#include "libcyclone.hpp"
#include "cyclone.hpp"

static const int SCHED_BUFFER_FULL  = 1;
static const int NON_COMMUTE = 2;

extern struct rte_ring ** to_cores;

typedef struct node_st{
	uint32_t id;
	unsigned long me_quorum;
	rte_mbuf *m;  // rpc and wal pointers are offset of m pointer. for each acccess
	rpc_t *rpc;
	wal_entry_t *wal;
	struct node_st *next;
	struct node_st *prev;
}node_t;



typedef struct scheduler_st{
	
	node_t *head;
	node_t *tail;
	node_t *next_schedule;
	uint32_t size;
	uint64_t id;	// epoch id
	unsigned int rb_counter; // round robin counter



int init(){
	head = (node_t *)rte_malloc("scheduler elem",sizeof(node_t),0);
	head->m = NULL;
	head->rpc = NULL;
	head->wal = NULL;
	head->next = NULL;
	head->prev = NULL;

	tail = (node_t *)rte_malloc("scheduler elem",sizeof(node_t),0);
	tail->m = NULL;
	tail->rpc = NULL;
	tail->wal = NULL;
	tail->next = NULL;
	tail->prev = NULL;

	next_schedule = NULL;

	head->next = tail;
	tail->prev = head;

	id = 0;
	size = 0;
	return 0;

}

int cleanup(){
	if(!size){
		return 0;
	}
	node_t *current_node = head->next;
	node_t *next_node;
	for(uint32_t i = 0; i <= size; i++){
		assert(current_node != NULL && "list node is null");
		next_node = current_node->next;
		rte_free(current_node);
		current_node = next_node;
	}
	head->next = tail;
	size = 0;
	return 0;
}

/*
 * add data operation in to our scheduling pool.
 */
int add(unsigned long me_quorum, rte_mbuf *m, rpc_t *rpc, wal_entry_t *wal){
	if(size == max_sched_buffer_length){
		return SCHED_BUFFER_FULL;
	}
	node_t *head = head;

	node_t *newest = (node_t *)rte_malloc("sched_node",sizeof(node_t),0);
	newest->me_quorum = me_quorum;	
	newest->m = m;
	newest->rpc = rpc;	
	
	node_t *recent = head->next;
	newest->next = recent;
	newest->prev = head;

	head->next = newest;
	recent->prev = newest;

	size++;
	return 0;
}


/*
 *  go through the buffered operations list and enqueue, ready operations
 *  in to exeuction threads 
 */
int schedule(op_commute_callback_t is_commute){
	node_t *list_node = next_schedule;
	while(list_node->prev != NULL){
		while(list_node->next != NULL){
			if(!list_node->wal->marked && !is_commute(next_schedule,list_node)){
				return NON_COMMUTE;
			}
			list_node = list_node->next;
		}
		/* enqueue in to exeucte threads */
		void *triple[3];
		triple[0] = (void *)(unsigned long)next_schedule->me_quorum;
		triple[1] = next_schedule->m;
		triple[2] = next_schedule->rpc;
		if(rte_ring_mp_enqueue_bulk(to_cores[rb_counter++%executor_threads], triple, 3) == -ENOBUFS) {
			BOOST_LOG_TRIVIAL(fatal) << "raft->core comm ring is full (req stable)";
			exit(-1);
		}

		next_schedule = next_schedule->prev;
		list_node = next_schedule;
	}
}

/*
 * free up memory of both scheduler list node and pointing mbuf structures.
 *
 */
int gc(){
	node_t *list_node = next_schedule->next; // most recent operation scheduled
	while(list_node->next != NULL){
		if(list_node->wal->marked){
			node_t *del_node = list_node;
			node_t *prev_node = list_node->prev;

			prev_node->next = list_node->next;
			list_node = list_node->next;
			list_node->prev = prev_node;

			rte_pktmbuf_free(del_node->m);
			rte_free(del_node);
			size--;
		}
	}
	return 0;
}

} scheduler_t;
#endif //CYCLONE_SCHEDULER_HPP

