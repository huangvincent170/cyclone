#include "pmemds-common.h"
#include "priority_queue/persistent_priority_queue.hpp"
#include "gtest/gtest.h"

namespace {

// The fixture for testing class Foo.
    class pqTest : public ::testing::Test {
    protected:

        /* Code here will be called immediately after the constructor (right
           before each test). */
        void SetUp() override {
            pq = new pmemds::persistent_priority_queue(4);
        }

        /* Code here will be called immediately after each test (right
           before the destructor). */
        void TearDown() override {
            delete(pq);
        }

        pmemds::persistent_priority_queue *pq;

    };

    TEST_F(pqTest, descendPrio) {
        struct vote_payload_st array[MAX_VOTE_PAYLOAD];
        int size;

        for(int i = 0; i < 10; i++) {
            pq->insert(i,10-i);
        }

        ASSERT_EQ(pq->read_topK(array,&size),0);
        for(int i = 0; i < size; i++){
            std::cout << std::to_string(array[i].idx) << " ";
        }
        std::cout << std::endl;
    }

    TEST_F(pqTest, ascendPrio) {
        struct vote_payload_st array[MAX_VOTE_PAYLOAD];
        int size;

        for(int i = 0; i < 10; i++) {
            pq->insert(i,i);
        }

        ASSERT_EQ(pq->read_topK(array,&size),0);
        for(int i = 0; i < size; i++){
            std::cout << std::to_string(array[i].idx) << " ";
        }
        std::cout << std::endl;

    }

    TEST_F(pqTest, decreasePrio) {
        struct vote_payload_st array[MAX_VOTE_PAYLOAD];
        int size;

        for(int i = 0; i < 10; i++) {
            pq->insert(i,i);
        }
        pq->decrease_prio(8,6);
        ASSERT_EQ(pq->read_topK(array,&size),0);
        for(int i = 0; i < size; i++){
            std::cout << std::to_string(array[i].idx) << " ";
        }
        std::cout << std::endl;

    }


    TEST_F(pqTest, increasePrio) {
        struct vote_payload_st array[MAX_VOTE_PAYLOAD];
        int size;

        for(int i = 0; i < 10; i++) {
            pq->insert(i,i);
        }
        pq->increase_prio(2,10);
        ASSERT_EQ(pq->read_topK(array,&size),0);
        for(int i = 0; i < size; i++){
            std::cout << std::to_string(array[i].idx) << " ";
        }
        std::cout << std::endl;

    }


/*

    TEST_F(pqTest, incrTest){
        unsigned long key, value;
        ASSERT_EQ(pq->increase_prio(3, 12),0);

        ASSERT_EQ(pq->get_max(key,value),0);
        ASSERT_EQ(key,3);
        ASSERT_EQ(value,22);
    }

    TEST_F(pqTest, decrTest){
        unsigned long key, value;
        ASSERT_EQ(pq->decrease_prio(9,5),0);
        ASSERT_EQ(pq->get_max(key,value),0);
        ASSERT_NE(key,9);
        ASSERT_NE(value,5);
    }

   */

}