#ifndef __PERSISTENT_PRIORITY_QUEUE_H
#define __PERSISTENT_PRIORITY_QUEUE_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "../pmemds.h"


#define MIN_HEAP 0
#define MAX_HEAP 1

#define K_VALUE  5  //top-k value

struct pqelem_st{
    pqelem_st(unsigned long k, unsigned long p):key(k),priority(p){
    }
    unsigned long key;
    unsigned long priority;
    unsigned int my_heap; // min/max heap
};

namespace pmemds {

    class persistent_priority_queue {
    public:
        persistent_priority_queue();
        ~persistent_priority_queue();

        int insert(unsigned long key, unsigned long priority);
        int remove(unsigned long key);

        int increase_prio(const unsigned long key, unsigned long delta_prio);
        int decrease_prio(const unsigned long key, unsigned long delta_prio);
        int read_topK();

        void printq();

    private:
        //TODO: without inline multiple definition? could not figure out why? Should work fine.
        void max_heapify( const unsigned long idx);
        unsigned long right_child(unsigned long idx);
        unsigned long left_child(unsigned long idx);
        unsigned long parent_of(unsigned long idx);
        void swap(unsigned long idx1, unsigned long idx2);

        unsigned long gtol(unsigned long idx){ //global to local index
            if(idx < K_VALUE) {
                return idx;
            }
            return idx-K_VALUE;
        };
        unsigned long mintog(unsigned long idx) { //min to global index
            return idx;
        };
        unsigned long maxtog(unsigned long idx) { //max to global index
            return idx+K_VALUE;
        };
        int is_minheap(unsigned long global_idx){
            return global_idx < K_VALUE;
        }

        void min_heapify(const unsigned long idx); // min heap maintain top K

        std::vector<struct pqelem_st *> *max_elems; // main data structure
        std::vector<struct pqelem_st *> *min_elems; // we maintain top K in a min heap.
        std::unordered_map<unsigned long,unsigned long> *keymap; // key to index mapping aux structure
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
        std::vector<struct pqelem_st*>::iterator vit;
        // heap itself indexed by the priority, hence first have to lookup the
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
            unsigned long local_index = gtol(g_idx);
            min_elems->at(local_index)->priority += delta_prio;
            min_heapify(local_index);
            return 0;
        }else{
            /*
             * 1. element in max-heap. increase priority. Move the element upward.
             * 2. If the max-heap root prio is > min-heap root, then move root elements around.
             * 3. max-heaps' new root will be swapped in. No heapify required. ( refer invariant )
             * 4. min-heaps' new root requires min-heapify.
             */
            unsigned long idx = gtol(g_idx);
            max_elems->at(idx)->priority += delta_prio;
            while(idx > 0 && max_elems->at(idx)->priority > max_elems->at(parent_of(idx))->priority){
                swap(idx,parent_of(idx));
                idx = parent_of(idx);
            }

            if(max_elems->at(0) > min_elems->at(0)){
                struct pqelem_st *temp = max_elems->at(0);
                max_elems->at(0) = min_elems->at(0);

                min_elems->at(0) = temp;
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
            unsigned long local_index = gtol(g_idx);
            max_elems->at(local_index)->priority -= delta_prio; //TBD: guard against zero prio
            max_heapify(local_index);
            return 0;

        }else{

            /*
             * 1. element in min-heap. Decrease priority and move the element to right place.
             * 2. If the min-heap root (updated element ended up here) is less than max-heap root,swap roots.
             * 3. replace min-heap root, and done.
             * 4. replace max-heap root and call max-heapify on it.
             */

        }


        max_elems->at(idx)->priority -= delta_prio;
        max_heapify(idx);
        return 0;
    }

    inline int persistent_priority_queue::insert(unsigned long key, unsigned long priority) {
        std::vector<struct pqelem_st*>::iterator it;
        struct pqelem_st *pqelem = new struct pqelem_st(key,priority);

        /// if min-heap is not filled, then fill it
        if(min_elems->size() < K_VALUE){
            min_elems->push_back(pqelem);
            unsigned long idx = min_elems->size()-1;
            keymap->insert(std::pair<unsigned long, unsigned long>(key,idx));
            while(idx > 0 && min_elems->at(idx)->priority < min_elems->at(parent_of(idx))->priority){
                swap(min_elems,idx,parent_of(idx));
                idx = parent_of(idx);
            }
            return 0;
        }

        /// if incoming element does not belong to top-K, then insert in to max-heap
        if(min_elems->at(0)->priority < priority) {
            max_elems->push_back(pqelem);
            unsigned long idx = max_elems->size() - 1;
            keymap->insert(std::pair<unsigned long, unsigned long>(key, idx));
            while (idx > 0 && max_elems->at(idx)->priority > max_elems->at(parent_of(idx))->priority) {
                swap(max_elems, idx, parent_of(idx));
                idx = parent_of(idx);
            }
            return 0;
        }

        /// otherwise, extract root element out of min-heap, and insert it in to max-heap.
        /// insert new element to min-heap root and min-heapify

        struct pqelem_st *minimum = min_elems->at(0);

        max_elems->push_back(minimum);
        unsigned long local_idx  = max_elems->size() - 1;
        keymap->insert(std::pair<unsigned long, unsigned long>(key, maxtog(local_idx)));
        while (idx > 0 && max_elems->at(idx)->priority > max_elems->at(parent_of(idx))->priority) {
            swap(max_elems, idx, parent_of(idx));
            idx = parent_of(idx);
        }

        std::unordered_map<unsigned long,unsigned long>::iterator  it = keymap->find(key);
        if(it == keymap->end()){
            LOG_ERROR("key not found");
            return -1;
        }
        it->second = mintog(0);
        min_elems->at(0) = minimum;
        min_heapify(0);

    }

    inline void persistent_priority_queue::max_heapify(const unsigned long idx) {
        unsigned long largest;
        unsigned long rchild_idx = right_child(idx);
        unsigned long lchild_idx = left_child(idx);

        if(rchild_idx < max_elems->size() && max_elems->at(rchild_idx)->priority > max_elems->at(idx)->priority){
            largest = rchild_idx;
        }else{
            largest = idx;
        }
        if(lchild_idx < max_elems->size() &&  max_elems->at(lchild_idx)->priority > max_elems->at(largest)->priority){
            largest = lchild_idx;
        }
        if(largest != idx){
            swap(largest,idx);
            max_heapify(largest);
        }
    }


    inline void persistent_priority_queue::min_heapify(const unsigned long idx) {
        unsigned long min;
        unsigned long rchild_idx = right_child(idx);
        unsigned long lchild_idx = left_child(idx);

        if(rchild_idx < max_elems->size() && max_elems->at(rchild_idx)->priority < max_elems->at(idx)->priority){
            min = rchild_idx;
        }else{
            min = idx;
        }
        if(lchild_idx < max_elems->size() &&  max_elems->at(lchild_idx)->priority < max_elems->at(largest)->priority){
            min = lchild_idx;
        }
        if(min != idx){
            swap(min,idx);
            min_heapify(min);
        }
    }
    
    
    inline int persistent_priority_queue::read_topK(){


    }

    inline void persistent_priority_queue::swap(std::vector<pqelem_st *> pq_vector, const unsigned long idx1,const unsigned long idx2) {
        std::vector<unsigned long>::iterator it1,it2;

        pqelem_st *elem1 = pq_vector->at(idx1);
        pqelem_st *elem2 = pq_vector->at(idx2);

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
        one->second = idx2;
        two->second = idx1;

        std::swap(pq_vector->at(idx1),pq_vector->at(idx2));
    }

    inline persistent_priority_queue::persistent_priority_queue(){
        this->max_elems = new std::vector<struct pqelem_st *>();
        this->keymap = new std::unordered_map<unsigned long, unsigned long>();
    }

    inline persistent_priority_queue::~persistent_priority_queue() {
        std::vector<pqelem_st *>::iterator it;
        for(it = max_elems->begin(); it != max_elems->end(); it++){
            delete(*it);
        }
        delete(max_elems);
        delete(keymap);
    }

    inline void persistent_priority_queue::printq() {
        std::vector<pqelem_st *>::iterator it;
        for(it = max_elems->begin(); it != max_elems->end(); it++){
            std::cout << (*it)->key << " : " << (*it)->priority << std::endl;
        }
    }

}

#endif

