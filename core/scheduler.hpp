/*
 * 
 * 
 */
#ifndef CYCLONE_SCHEDULER_QUEUE_HPP
#define CYCLONE_SCHEDULER_QUEUE_HPP

#include <rte_mbuf.h>
#include <rte_malloc.h>

#include "libcyclone.hpp"


static const NO_FREE_ELEMS 
static const SET_EMPTY 
static const NOT_FOUND


typedef struct node_st{
	uint32_t id;
	rte_mbuf *m; 
	struct node_st *next;
	struct node_st *prev;
}node_t;



typedef struct scheduler_st{
    node_t *head;
    node_t *tail;
    node_t *next_schedule;
	uint32_t size;
	uint64_t id;	// epoch id
} scheduler_t;



int init_scheduler(scheduler_t *handle){
	handle->head = (node_t *)rte_malloc("scheduler elem",sizeof(node_t));
	head->data = NULL;
	head->next = NULL;
	head->prev = NULL;

	handle->tail = (node_t *)rte_malloc("scheduler elem",sizeof(node_t));
	tail->data = NULL;
	tail->next = NULL;
	tail->prev = NULL;

	handle->next_schedule = NULL
	handle->id = 0;
	handle->size = 0;
	return 0;
	
}

int close_scheduler(scheduler_t *handle){
	if(!handle->size){
	   return 0;
	}	   
    node_t *current_node = head->next;	
	node_t *next_node;
	for(uint32_t i = 0; i <= size; i++){
		assert(current_node != NULL && "list node is null");
		next_node = current_node->next;
		free(current_node);
		current_node = next_node;	
	}
	handle->head = head->tail;
	handle->size = 0;
	return 0;
}

/*
 * add data operation in to our scheduling pool.
 */
int scheduler_add(scheduler_t *handle, ){
	if(handle->size == max_sched_buffer_length){
		return SCHED_BUFFER_FULL;
	}
	node_t *head = handle->head;
	node_t *newest = (node_t *)rte_malloc("sched_node",sizeof(node_t));
	node_t *recent = head->next;

	newest->next = recent;
	newest->prev = head;

	head->next = newest;
	recent->prev = neweset;

	handle->size++
	return 0;
}


/*
 *
 */ 
int scheduler_schedule(scheduler_t *handle, int (*commute)(void *, void *)){


}

/*
 *
 */ 
int scheduler_gc(scheduler_t *handle){

}
#endif //CYCLONE_SCHEDULER_HPP

