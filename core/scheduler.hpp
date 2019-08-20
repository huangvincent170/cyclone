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
#include "logging.hpp"

static const int SCHED_BUFFER_FULL  = 1;
static const int NON_COMMUTE = 2;

extern struct rte_ring ** to_cores;

typedef struct node_st{
	uint32_t id;
	int to_core;
	unsigned long me_quorum;
	rte_mbuf *m;  // rpc and wal pointers are offset of m pointer
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
	BOOST_LOG_TRIVIAL(info) << "shcduler starting....";
	head = (node_t *)rte_malloc("scheduler elem",sizeof(node_t),0);
	if(head == NULL){
		BOOST_LOG_TRIVIAL(error) << "bad memory alloc";
		exit(-1);
	}
	head->m = NULL;
	head->rpc = NULL;
	head->wal = NULL;
	head->next = NULL;
	head->prev = NULL;

	tail = (node_t *)rte_malloc("scheduler elem",sizeof(node_t),0);
	if(tail == NULL){
		BOOST_LOG_TRIVIAL(error) << "bad memory alloc";
		exit(-1);
	}
	tail->m = NULL;
	tail->rpc = NULL;
	tail->wal = NULL;
	tail->next = NULL;
	tail->prev = NULL;

	head->next = tail;
	tail->prev = head;
	
	next_schedule = head;

	id = 0;
	size = 0;
	rb_counter=0;
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
int add(int to_core, unsigned long me_quorum, rte_mbuf *m, rpc_t *rpc, wal_entry_t *wal){
	if(size == max_sched_buffer_length){
		return SCHED_BUFFER_FULL;
	}
	node_t *newest = (node_t *)rte_malloc("sched_node",sizeof(node_t),0);
	if(newest == NULL){
		BOOST_LOG_TRIVIAL(error) << "bad memory alloc";
		exit(-1);
	}

	newest->to_core = to_core;
	newest->me_quorum = me_quorum;	
	newest->m = m;
	newest->rpc = rpc;	
	newest->wal = wal;
	
	node_t *top_node = head->next;
	newest->next = top_node;
	newest->prev = head;

	head->next = newest;
	top_node->prev = newest;
	if(next_schedule == head){
		next_schedule = newest;  // no operations to schedule
	}
	//BOOST_LOG_TRIVIAL(info) << "scheduler add,  list size "<< size <<" quorum : " << me_quorum << " m :" << m << " rpc : " << rpc;
	
	size++;
	return 0;
}


/*
 *  go through the buffered operations list and enqueue, ready operations
 *  in to exeuction threads 
 */
int schedule(op_commute_callback_t is_commute, op_partition_callback my_partition){
	//BOOST_LOG_TRIVIAL(info) << "schdule attempt...";
	node_t *list_node = next_schedule->next;
	while(next_schedule != head){
		while(list_node->next != NULL){ // tail node
			void *next_op = (void *)(next_schedule->rpc+1);
			void *list_op = (void *)(list_node->rpc+1);
			if(!list_node->wal->marked && !is_commute(next_schedule->rpc->core_mask, next_op, 
						list_node->rpc->core_mask, list_op)){
//	BOOST_LOG_TRIVIAL(info) << "scheduler return, non-commute, " 
//					<< " makred for gc : " << list_node->wal->marked
//					<< " buffer size : " << size;
				return NON_COMMUTE;
			}
			list_node = list_node->next;
		}
		/* enqueue in to exeucte threads */
		void *triple[3];
		triple[0] = (void *)(unsigned long)next_schedule->me_quorum;
		triple[1] = next_schedule->m;
		triple[2] = next_schedule->rpc;
#ifdef __PARTITION
		uint8_t partition = 0;
		if((parition = my_partition(next_schedule->rpc)) >= executor_threads){
			BOOST_LOG_TRIVIAL(error) << "partition id is greater than available executors";
			exit(-1);
		}
		if(rte_ring_mp_enqueue_bulk(to_cores[partition]], triple, 3) == -ENOBUFS) {
#else
		if(rte_ring_mp_enqueue_bulk(to_cores[rb_counter++%executor_threads], triple, 3) == -ENOBUFS) {
#endif
			BOOST_LOG_TRIVIAL(fatal) << "raft->core comm ring is full (req stable)";
			exit(-1);
		}
//	BOOST_LOG_TRIVIAL(info) << "scheduler enqueue , list size: " << size << " quorum : " << next_schedule->me_quorum << " m :" << next_schedule->m << " rpc : " << next_schedule->rpc;

		next_schedule = next_schedule->prev;
		list_node = next_schedule->next;
	}
	//BOOST_LOG_TRIVIAL(info) << "schduler return.";
	return 0;
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

			rte_pktmbuf_free(del_node->m); //TODO: persistent pointer from volatile structure. Must revisit
			rte_free(del_node);
			size--;
			//BOOST_LOG_TRIVIAL(info) << "gc done , list size: " << size ;
		}else{
			list_node = list_node->next;
		}
	}
	//BOOST_LOG_TRIVIAL(info) << "gc done , list size: " << size ;
	return 0;
}

} scheduler_t;
#endif //CYCLONE_SCHEDULER_HPP

