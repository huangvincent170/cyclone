#ifndef __PERSISTENT_PRIORITY_QUEUE_H
#define __PERSISTENT_PRIORITY_QUEUE_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "../pmemds.h"

struct pqelem_st{
    pqelem_st(unsigned long k, unsigned long p):key(k),priority(p){
    }
    unsigned long key;
    unsigned long priority;
};

namespace pmemds {

    class persistent_priority_queue {
    public:

        persistent_priority_queue();

        ~persistent_priority_queue();
        int insert(unsigned long key, unsigned long priority);

        int increase_prio(const unsigned long key, unsigned long delta_prio);
        int decrease_prio(const unsigned long key, unsigned long delta_prio);
        int get_max(unsigned long &key, unsigned long &priority);
        void printq();

    private:
        //TODO: without inline multiple definition? could not figure out why? Should work fine.
        void max_heapify( const unsigned long idx);
        unsigned long right_child(unsigned long idx);
        unsigned long left_child(unsigned long idx);
        unsigned long parent_of(unsigned long idx);
        void swap(unsigned long idx1, unsigned long idx2);


        std::vector<struct pqelem_st *> *elems;
        std::unordered_map<unsigned long,unsigned long> *keymap;
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
        unsigned long idx = it->second;
        elems->at(idx)->priority += delta_prio;
        // push up the element
        while(idx > 0 && elems->at(idx)->priority > elems->at(parent_of(idx))->priority){
            swap(idx,parent_of(idx));
            idx = parent_of(idx);
        }
        return 0;
    }

    inline int persistent_priority_queue::decrease_prio(const unsigned long key, unsigned long delta_prio) {
        // heap itself indexed by the priority, hence first have to lookup the
        std::unordered_map<unsigned long,unsigned long>::iterator  it = keymap->find(key);
        if(it == keymap->end()){
            LOG_ERROR("key not found");
            return -1;
        }
        unsigned long idx = it->second;
        elems->at(idx)->priority -= delta_prio;
        max_heapify(idx);
        return 0;
    }

    inline int persistent_priority_queue::insert(unsigned long key, unsigned long priority) {
        std::vector<struct pqelem_st*>::iterator it;
        struct pqelem_st *pqelem = new struct pqelem_st(key,priority);

        elems->push_back(pqelem);
        unsigned long idx = elems->size()-1;
        keymap->insert(std::pair<unsigned long, unsigned long>(key,idx));
        while(idx > 0 && elems->at(idx)->priority > elems->at(parent_of(idx))->priority){
            swap(idx,parent_of(idx));
            idx = parent_of(idx);
        }

        return 0;
    }

    inline void persistent_priority_queue::max_heapify(const unsigned long idx) {
        unsigned long largest;
        unsigned long rchild_idx = right_child(idx);
        unsigned long lchild_idx = left_child(idx);

        if(rchild_idx < elems->size() && elems->at(rchild_idx)->priority > elems->at(idx)->priority){
            largest = rchild_idx;
        }else{
            largest = idx;
        }
        if(lchild_idx < elems->size() &&  elems->at(lchild_idx)->priority > elems->at(largest)->priority){
            largest = lchild_idx;
        }
        if(largest != idx){
            swap(largest,idx);
            max_heapify(largest);
        }
    }

    inline int persistent_priority_queue::get_max(unsigned long &key, unsigned long &priority) {
        struct pqelem_st *max = elems->at(0);
        key = max->key;
        priority = max->priority;
        keymap->erase(max->key);
        delete(max);

        pqelem_st *last = elems->back();
        elems->at(0) = last;
        elems->pop_back();
        if(elems->size() > 1) {
            auto first = keymap->find(last->key);
            if (first == keymap->end()) {
                LOG_ERROR("key  not found");
                return -1;
            }
            first->second = 0;

            max_heapify(0); // begin index
        }
        //printq();
        return 0;

    }

    inline void persistent_priority_queue::swap(const unsigned long idx1,const unsigned long idx2) {
        std::vector<unsigned long>::iterator it1,it2;

        pqelem_st *elem1 = elems->at(idx1);
        pqelem_st *elem2 = elems->at(idx2);

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

        std::swap(elems->at(idx1),elems->at(idx2));
    }

    inline persistent_priority_queue::persistent_priority_queue(){
        this->elems = new std::vector<struct pqelem_st *>();
        this->keymap = new std::unordered_map<unsigned long, unsigned long>();
    }

    inline persistent_priority_queue::~persistent_priority_queue() {
        std::vector<pqelem_st *>::iterator it;
        for(it = elems->begin(); it != elems->end(); it++){
            delete(*it);
        }
        delete(elems);
        delete(keymap);
    }

    inline void persistent_priority_queue::printq() {
        std::vector<pqelem_st *>::iterator it;
        for(it = elems->begin(); it != elems->end(); it++){
            std::cout << (*it)->key << " : " << (*it)->priority << std::endl;
        }
    }

}

#endif

