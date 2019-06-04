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
            pq = new pmemds::persistent_priority_queue();
            for(int i = 0; i < 10; i++) {
                pq->insert(i,10);
            }

        }

        /* Code here will be called immediately after each test (right
           before the destructor). */
        void TearDown() override {
            delete(pq);
        }

        pmemds::persistent_priority_queue *pq;

    };

    TEST_F(pqTest, readTest) {
        unsigned long key, value;
        ASSERT_EQ(pq->get_max(key,value),0);
        ASSERT_EQ(key,0);
        ASSERT_EQ(value,10);


        ASSERT_EQ(pq->get_max(key,value),0);
        ASSERT_EQ(key,9);
        ASSERT_EQ(value,10);

        ASSERT_EQ(pq->get_max(key,value),0);
        ASSERT_EQ(key,8);
        ASSERT_EQ(value,10);
    }


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

    /*TEST_F(pqTest,maxTest){
        for(int i = 11; i < 20; i++){
            votes->insert(i,i);
        }
    }*/

}