#include <iostream>
#include <vector>
#include <unordered_map>

#include "../pmemds.h"



typedef struct pqelem_st{
    unsigned long key;
    unsigned long priority;
};


namespace pmemds {

    class persistent_priority_queue {
    public:

        persistent_priority_queue();

        ~persistent_priority_queue();

        PMStatus insert(unsigned long key, unsigned long priority);

        PMStatus increase_prio(const unsigned &key, unsigned long &delta_prio);
        PMStatus decrease_prio(const unsigned& key, unsigned long &delta_prio);

        PMStatus extract_max(unsigned long &key, unsigned long &priority);

    private:
        void max_heapify( const std::vector<struct pqelem_st *>::iterator it);
        std::vector<struct pqelem_st *>::iterator right_child(std::vector<struct pqelem_st *>::iterator  it);
        std::vector<struct pqelem_st *>::iterator left_child(std::vector<struct pqelem_st *>::iterator  it);
        std::vector<struct pqelem_st *>::iterator parent_of(std::vector<struct pqelem_st *>::iterator  it);

        std::vector<struct pqelem_st *> *elems;
        std::unordered_map<unsigned long,unsigned long> *keymap;
    };


    std::vector<struct pqelem_st *>::iterator persistent_priority_queue::right_child(std::vector<struct pqelem_st *>::iterator &it) {
        return elems->begin() +  2*std::distance(it,elems->begin());
    }

    std::vector<struct pqelem_st *>::iterator persistent_priority_queue::left_child(std::vector<struct pqelem_st *>::iterator &it) {
        return elems->begin() +  (2*std::distance(it,elems->begin()) + 1);
    }

    std::vector<struct pqelem_st *>::iterator persistent_priority_queue::parent_of(std::vector<struct pqelem_st *>::iterator  it){

    }



    PMStatus persistent_priority_queue::increase_prio(const unsigned &key, unsigned long &delta_prio) {
        std::vector<struct pqelem_st*>::iterator vit;
        // heap itself indexed by the priority, hence first have to lookup the
        std::unordered_map<unsigned long,unsigned long>::iterator  it = keymap->find(key);
        if(it == keymap->end()){
            LOG_ERROR("key not found");
            return FAILED;
        }
        unsigned long distance = it->second;
        vit = elems->begin();
        std::advance(vit,distance);
        // push up the element


    }

    PMStatus persistent_priority_queue::decrease_prio(const unsigned &key, unsigned long &delta_prio) {
        std::vector<struct pqelem_st*>::iterator vit;
        // heap itself indexed by the priority, hence first have to lookup the
        std::unordered_map<unsigned long,unsigned long>::iterator  it = keymap->find(key);
        if(it == keymap->end()){
            LOG_ERROR("key not found");
            return FAILED;
        }
        unsigned long distance = it->second;
        vit = elems->begin();
        std::advance(vit,distance);
        //push element down
        max_heapify(vit);

    }

    PMStatus persistent_priority_queue::insert(unsigned long key, unsigned long priority) {
            std::vector<struct pqelem_st*>::iterator it;
            struct pqelem_st *pqelem = new struct pqelem_st(key,priority);
            elems->push_back(pqelem);
        it = elems->end();
        while(it >= elems->begin() && (*parent_of(it))->priority < (*it)->priority){
            swap(it,parent_of(it));
            it = parent_of(it);
        }
    }

    void persistent_priority_queue::max_heapify(const std::vector<struct pqelem_st *>::iterator it) {
        std::vector<struct pqelem_st *>::iterator largest;
        std::vector<struct pqelem_st *>::iterator r_child = right_child(it);
        std::vector<struct pqelem_st *>::iterator l_child = left_child(it);

        if((*r_child)->priority > (*it)->priority){
            largest = r_child;
        }else{
            largest = it;
        }
        if((*l_child)->priority > (*largest)->priority){
            largest = l_child;
        }
        if(largest != it){
            swap(largest,it);
            max_heapify(largest);
        }
    }

    PMStatus persistent_priority_queue::extract_max(unsigned long &key, unsigned long &priority) {
        struct pqelem_st *max = *elems->begin();
        *(elems->begin()) = *(elems->end());
        elems->erase(elems->end());
        max_heapify(elems->begin());

        key = max->key;
        priority = max->priority;

        return OK;

    }


}



