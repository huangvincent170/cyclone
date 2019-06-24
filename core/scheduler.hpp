/*
 * rte_ring implementation provided by DPDK does not allow up to
 * traverse the queue content
 */
#ifndef CYCLONE_SCHEDULER_QUEUE_HPP
#define CYCLONE_SCHEDULER_QUEUE_HPP

#include "libcyclone.hpp"
#include "mcs_lock.hpp"


extern static __thread struct mcs_node local_node;

static const NO_FREE_ELEMS 
static const SET_EMPTY 
static const NOT_FOUND



typedef struct node_st{
	uint32_t id;
	uint8_t marker;
	void *data;
	struct node_st *next;
}node_t;

typedef scheduler_set_st{
    node_t *head;
    node_t *tail;
	uint32_t size;
	uint64_t id;	// epoch id
	mcs_lock qlock; // spinlock based on MCS lock
} scheduler_set_t;

int create_scheduler_set(scheduler_set_t *handle){
	handle->head = malloc(sizeof(node_t));
	head->data = NULL;
	head->next = NULL;

	handle->tail = malloc(sizeof(node_t));
	tail->data = NULL;
	tail->next = NULL;

	handle->id = NULL;
	handle->id = 0;
	handle->size = 0;

	return 0;
	
}

int delete_scheduler_set(scheduler_set_t *handle){
	spin_lock(&handle->qlock);
	if(!handle->size){
	   spin_unlock(&handle->qlock);
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
	spin_unlock(&handle->qlock);
	return 0;
}

/*
 * enqueue data operation in to our scheduling pool.
 * We check incoming operation with each of the current queued operations
 * for commutativity.
 * 1. operation is added to the pool if it commutes
 * 2. If not, wait till conflicting operation/s retires.
 *
 */
int insert_op(scheduler_set_t *handle, void *ptr, int (*commute)(void *, void *)){
	spin_lock(&handle->qlock);
	if(handle->size == max_ ){
	   spin_unlock(&handle->qlock);
		return SET_FULL;
	}
	
	node_t *node = handle->head;
	while((node = node->next) != NULL){
		if(!commute(ptr,node->data)){
			spin_unlock(&handle->qlock);
			return NO_COMMUTE;
		}
	}
	//add to tail
    node_t *new_tail = malloc(sizeof(node_t));
	new_tail->data = NULL;
	new_tail->next = NULL;

	tail->data = ptr;
	tail->marked = 0;
	tail->id = ++handle;
	tail->next = new_tail;

	handle->size++

	spin_unlock(&handle->qlock);
	return 0;
}


/*
 * pick the oldest operation to execute.
 */
int pick_op(scheduler_set_t *handle,  void **ptr){
  spin_lock(&handle->qlock);
  if(!handle->size) {
	spin_unlock(&handle->qlock);
	return SET_EMPTY;
  }

  node_t *node = handle->head;
  while((node = node->next) != NULL){
	if(!node->marked){ 
		node->marked = TRUE;
		*ptr = node;
		spin_unlock(&handle->qlock);
		return 0;
	}
  }	  
  spin_unlock(&handle->qlock);
  return NO_FREE_ELEMS; 

}

/* 
 * mark the operation as executed
 */ 
int retire_op(scheduler_set_t *handle, uint32_t id){
	assert(handle->size && "empty set");
	
	spin_lock(&handle->qlock);
	node_t *node = handle->head;
	node_t *temp;
	while(node->next != NULL){
		if(node->next->id == id){
			node_t *retired = node->next;	
			node->next = node->next->next;
			free(retired);
			handle->size--;
			spin_unlock(&handle->qlock);
			return 0;
		}
	}
	assert(0 && "wrong control flow");
	spin_unlock(&handle->qlock);
	return NOT_FOUND;
}

#endif //CYCLONE_SCHEDULER_HPP

