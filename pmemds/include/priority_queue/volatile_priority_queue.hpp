#ifndef __VOLATILE_PRIORITY_QUEUE_H
#define __VOLATILE_PRIORITY_QUEUE_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "../pmemds.h"


#define MIN_HEAP 0
#define MAX_HEAP 1

#define NTOP_K  8  // k value in top-k

struct pqelem_st{
    pqelem_st(unsigned long k, unsigned long p):key(k),priority(p){
    }
    unsigned long key;
    unsigned long priority;
};


/* Total index space is formatted as below.
|(index 0 to k-1) <-- min heap --> | (k to infinity) <-- max heap --> |

*/

namespace pmemds {

    class persistent_priority_queue {
    public:
        persistent_priority_queue(int npartitions); // we tune the min-heap size based on number of partitions
        ~persistent_priority_queue();

        int insert(unsigned long key, unsigned long priority);
        int remove(unsigned long key);

        int increase_prio(const unsigned long key, unsigned long delta_prio);
        int decrease_prio(const unsigned long key, unsigned long delta_prio);
        int read_topK(struct vote_payload_st *array, int *size);

        void printq();

    private:
		int minheap_size;	

        //TODO: without inline multiple definition? could not figure out why? Should work fine.
        void max_heapify( const unsigned long idx);
        unsigned long right_child(unsigned long idx);
        unsigned long left_child(unsigned long idx);
        unsigned long parent_of(unsigned long idx);
        void swap(std::vector<struct pqelem_st *> *elems, uint8_t heap_type, unsigned long idx1, unsigned long idx2);

        unsigned long gtol(unsigned long idx){ //global to local index
            if(idx < minheap_size) {
                return idx;
            }
            return idx-minheap_size;
        };
        unsigned long mintog(unsigned long idx) { //min to global index
            return idx;
        };
        unsigned long maxtog(unsigned long idx) { //max to global index
            return idx+minheap_size;
        };
        int is_minheap(unsigned long global_idx){
            return global_idx < minheap_size;
        }

        void min_heapify(const unsigned long idx); // min heap maintain top K

        std::vector<struct pqelem_st *> *min_elems; // we maintain top K in a min heap.
        std::vector<struct pqelem_st *> *max_elems; // rest of the data in max heap
        std::unordered_map<unsigned long,unsigned long> *keymap; // key to global_index mapping auxiliary structure

    };


    inline unsigned long persistent_priority_queue::right_child(unsigned long idx) {
        return 2*idx + 1;
    }

    inline unsigned long persistent_priority_queue::left_child(unsigned long idx) {
        return 2*idx + 2;
    }

    inline unsigned long persistent_priority_queue::parent_of(unsigned long idx){
        return idx/2;
    }


    inline int persistent_priority_queue::increase_prio(const unsigned long key, unsigned long delta_prio) {
        std::unordered_map<unsigned long,unsigned long>::iterator  it = keymap->find(key);
        if(it == keymap->end()){
            LOG_ERROR("key not found");
            return -1;
        }
        unsigned long g_idx = it->second;

        if(is_minheap(g_idx)){ // the data is in min-heap
            /*
             * 1. increase priority. It will not affect the max-heap
             * 2. min-heapify
             */
            unsigned long l_index = gtol(g_idx);
            min_elems->at(l_index)->priority += delta_prio;
            min_heapify(l_index);
            return 0;
        }else{
            /* 1. element in max-heap. increase priority. Move the element upward.
             * 2. If the max-heap root prio is > min-heap root, then move root elements around.
             * 3. max-heaps' new root will be swapped in. No heapify required. ( refer invariant )
             * 4. min-heaps' new root requires min-heapify.
             */
            unsigned long l_idx = gtol(g_idx);
            max_elems->at(l_idx)->priority += delta_prio;
            while(l_idx > 0 && max_elems->at(l_idx)->priority > max_elems->at(parent_of(l_idx))->priority){
                swap( max_elems,MAX_HEAP,l_idx,parent_of(l_idx)); /// TBD
                l_idx = parent_of(l_idx);
            }

            if(max_elems->at(0)->priority > min_elems->at(0)->priority){
                struct pqelem_st *temp = max_elems->at(0);

                max_elems->at(0) = min_elems->at(0);
                std::unordered_map<unsigned long,unsigned long>::iterator  it = keymap->find(min_elems->at(0)->key);
                if(it == keymap->end()){
                    LOG_ERROR("key not found");
                    return -1;
                }
                it->second = maxtog(0);

                min_elems->at(0) = temp;
                it = keymap->find(temp->key);
                if(it == keymap->end()){
                    LOG_ERROR("key not found");
                    return -1;
                }
                it->second = mintog(0);
                min_heapify(0);
                return 0;
            }

            return 0;
        }

    }

    inline int persistent_priority_queue::decrease_prio(const unsigned long key, unsigned long delta_prio) {
        // heap itself indexed by the priority, hence first have to lookup the
        std::unordered_map<unsigned long,unsigned long>::iterator  it = keymap->find(key);
        if(it == keymap->end()){
            LOG_ERROR("key not found");
            return -1;
        }
        unsigned long g_idx = it->second;
        if(!is_minheap(g_idx)){
            /*
             * 1. element in max-heap. Decrease priority. It will not affect the min-heap
             * 2. max-heapify at the updated element.
             */
            unsigned long l_index = gtol(g_idx);
            max_elems->at(l_index)->priority -= delta_prio; /// TBD: guard against zero prio
            max_heapify(l_index);
            return 0;

        }else{
            /* 1. element in min-heap. Decrease priority and move the element to right place.
             * 2. If the min-heap root (updated element ended up here) is less than max-heap root,swap roots.
             * 3. replace min-heap root, and done.
             * 4. replace max-heap root and call max-heapify on it.
             */
            unsigned long l_idx = gtol(g_idx);
            min_elems->at(l_idx)->priority -= delta_prio;
            while(l_idx > 0 && min_elems->at(l_idx)->priority < min_elems->at(parent_of(l_idx))->priority){
                swap( min_elems,MIN_HEAP,l_idx,parent_of(l_idx));
                l_idx = parent_of(l_idx);
            }

            if(min_elems->at(0)->priority < max_elems->at(0)->priority){
                struct pqelem_st *temp = min_elems->at(0);

                min_elems->at(0) = max_elems->at(0);
                std::unordered_map<unsigned long,unsigned long>::iterator  it = keymap->find(max_elems->at(0)->key);
                if(it == keymap->end()){
                    LOG_ERROR("key not found");
                    return -1;
                }
                it->second = mintog(0);

                max_elems->at(0) = temp;
                it = keymap->find(temp->key);
                if(it == keymap->end()){
                    LOG_ERROR("key not found");
                    return -1;
                }
                it->second = maxtog(0);
                max_heapify(0);
                return 0;
            }

            return 0;
        }
    }

    inline int persistent_priority_queue::insert(unsigned long key, unsigned long priority) {
        std::unordered_map<unsigned long,unsigned long>::iterator  it;
        struct pqelem_st *pqelem = new struct pqelem_st(key,priority);

        /// if min-heap is not filled, then fill it
        if(min_elems->size() < minheap_size){
            min_elems->push_back(pqelem);
            unsigned long l_idx = min_elems->size()-1;
            keymap->insert(std::pair<unsigned long, unsigned long>(key,mintog(l_idx)));
            while(l_idx > 0 && min_elems->at(l_idx)->priority < min_elems->at(parent_of(l_idx))->priority){
                swap(min_elems,MIN_HEAP,l_idx,parent_of(l_idx));
                l_idx = parent_of(l_idx);
            }
            return 0;
        }

        /// if incoming element does not belong to top-K, then insert in to max-heap
        if(min_elems->at(0)->priority > priority) {
            max_elems->push_back(pqelem);
            unsigned long l_idx = max_elems->size() - 1;
            keymap->insert(std::pair<unsigned long, unsigned long>(key, maxtog(l_idx)));
            while (l_idx > 0 && max_elems->at(l_idx)->priority > max_elems->at(parent_of(l_idx))->priority) {
                swap(max_elems, MAX_HEAP, l_idx, parent_of(l_idx));
                l_idx = parent_of(l_idx);
            }
            return 0;
        }

        /// otherwise, extract root element out of min-heap, and insert it in to max-heap.
        /// insert new element to min-heap root and min-heapify

        struct pqelem_st *minimum = min_elems->at(0);

        max_elems->push_back(minimum);
        unsigned long l_idx  = max_elems->size() - 1;
        it = keymap->find(minimum->key);
        if(it == keymap->end()){
            LOG_ERROR("key not found");
            return -1;
        }
        it->second = maxtog(l_idx);

        while (l_idx > 0 && max_elems->at(l_idx)->priority > max_elems->at(parent_of(l_idx))->priority) {
            swap(max_elems, MAX_HEAP, l_idx, parent_of(l_idx));
            l_idx = parent_of(l_idx);
        }
        /// insert to vacated min-root
        keymap->insert(std::pair<unsigned long, unsigned long>(key, mintog(0)));
        min_elems->at(0) = pqelem;
        min_heapify(0);
        return 0;
    }

    inline void persistent_priority_queue::max_heapify(const unsigned long idx) {
        unsigned long max;
        unsigned long rchild_idx = right_child(idx);
        unsigned long lchild_idx = left_child(idx);

        if(rchild_idx < max_elems->size() && max_elems->at(rchild_idx)->priority > max_elems->at(idx)->priority){
            max = rchild_idx;
        }else{
            max = idx;
        }
        if(lchild_idx < max_elems->size() &&  max_elems->at(lchild_idx)->priority > max_elems->at(max)->priority){
            max = lchild_idx;
        }
        if(max != idx){
            swap(max_elems,MAX_HEAP,max,idx);
            max_heapify(max);
        }
    }


    inline void persistent_priority_queue::min_heapify(const unsigned long idx) {
        unsigned long min;
        unsigned long rchild_idx = right_child(idx);
        unsigned long lchild_idx = left_child(idx);

        if(rchild_idx < min_elems->size() && min_elems->at(rchild_idx)->priority < min_elems->at(idx)->priority){
            min = rchild_idx;
        }else{
            min = idx;
        }
        if(lchild_idx < min_elems->size() &&  min_elems->at(lchild_idx)->priority < min_elems->at(min)->priority){
            min = lchild_idx;
        }
        if(min != idx){
            swap(min_elems,MIN_HEAP,min,idx);
            min_heapify(min);
        }
    }
    
    
    inline int persistent_priority_queue::read_topK(struct vote_payload_st *array, int *size){
        int nelems = min_elems->size();
        for(int i =0; i < nelems; i++){
            array[i].idx = min_elems->at(i)->key;
        }
        *size = nelems;
        return 0;
    }

    /*swap elements in a given vector. indexes are local */
    inline void persistent_priority_queue::swap(std::vector<struct pqelem_st *> *pq_vector,uint8_t heap_type, const unsigned long idx1,const unsigned long idx2) {
        std::vector<unsigned long>::iterator it1,it2;

        struct pqelem_st *elem1 = pq_vector->at(idx1);
        struct pqelem_st *elem2 = pq_vector->at(idx2);

        auto one = keymap->find(elem1->key);
        if(one == keymap->end()){
            LOG_ERROR("key one not found");
            return;
        }
        auto two = keymap->find(elem2->key);
        if(two == keymap->end()){
            LOG_ERROR("key two not found");
            return;
        }
        one->second = (heap_type == MIN_HEAP)?  mintog(idx2):maxtog(idx2);
        two->second = (heap_type == MIN_HEAP)?  mintog(idx1):maxtog(idx1);

        std::swap(pq_vector->at(idx1),pq_vector->at(idx2));
    }

    inline persistent_priority_queue::persistent_priority_queue(int npartitions){
		if(NTOP_K % npartitions){
			LOG_ERROR("number of partitions should be compatible with selected top-k value");
		}
		this->minheap_size = NTOP_K/npartitions;
        this->min_elems = new std::vector<struct pqelem_st *>();
        this->max_elems = new std::vector<struct pqelem_st *>();
        this->keymap = new std::unordered_map<unsigned long, unsigned long>();
    }

    inline persistent_priority_queue::~persistent_priority_queue() {
        std::vector<struct pqelem_st *>::iterator it;
        for(it = max_elems->begin(); it != max_elems->end(); it++){
            delete(*it);
        }
        delete(max_elems);
        delete(keymap);
    }

    inline void persistent_priority_queue::printq() {
        std::vector<struct pqelem_st *>::iterator it;
        for(it = max_elems->begin(); it != max_elems->end(); it++){
            std::cout << (*it)->key << " : " << (*it)->priority << std::endl;
        }
    }

}

#endif

