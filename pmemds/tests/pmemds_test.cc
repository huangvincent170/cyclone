
#include <string>

#include "../include/pmemds-client.h"
#include "../include/tree/btree-client.h"
#include "../include/priority_queue/priority_queue-client.h"
#include "gtest/gtest.h"


namespace {

// The fixture for testing class Foo.
    class pmemdsTest : public ::testing::Test {
    protected:
        const uint16_t  btree1 = 0;
        const uint16_t  btree2 = 1;
        const uint16_t  pq1 = 2;


        /* Code here will be called immediately after the constructor (right
           before each test). */
        void SetUp() override {

            /* server side data-structure creation */
            pmLib = new pmemds::PMLib();

            testClient = new pmemdsclient::TestClient(pmLib,&request,&response);
            testClient->open("testApp"); // TODO: move to pmlib
        }

        /* Code here will be called immediately after each test (right
           before the destructor). */
        void TearDown() override {
            testClient->close();
            delete(testClient);
            delete(pmLib);
        }

        // Objects declared here can be used by all tests in the test case for Foo.
        pm_rpc_t request,response;
        pmemdsclient::TestClient *testClient;
        pmemds::PMLib *pmLib;


    };

    TEST_F(pmemdsTest, BasicBTreeRWTest) {
        pmemdsclient::BTreeEngine *bte = new pmemdsclient::BTreeEngine(testClient,btree1,1024*1024*8,0UL);

        ASSERT_EQ(bte->create(PM_CREAT),OK);
        //TODO: check state of request and response
        ASSERT_EQ(bte->put(1234UL,"test_1234"),OK);
        //std::string str = "test_1234";
        ASSERT_STREQ(bte->get(1234UL).c_str(),"test_1234");
        ASSERT_EQ(bte->close(),OK);
        ASSERT_EQ(bte->remove(),OK);
    }


    /* request routed to proper data-structure? */
    TEST_F(pmemdsTest, MultipleBTreeTest) {
        pmemdsclient::BTreeEngine *bt1 = new pmemdsclient::BTreeEngine(testClient,btree1,100,0UL);
        pmemdsclient::BTreeEngine *bt2 = new pmemdsclient::BTreeEngine(testClient,btree2,100,0UL);

        ASSERT_EQ(bt1->create(PM_CREAT),OK);
        ASSERT_EQ(bt2->create(PM_CREAT),OK);

        ASSERT_EQ(bt1->put(1234UL,"test_bt1"),OK);
        ASSERT_EQ(bt2->put(1235UL,"test_bt2"),OK);

        ASSERT_STREQ(bt2->get(1235UL).c_str(),"test_bt2");
        ASSERT_STREQ(bt1->get(1234UL).c_str(),"test_bt1");

        ASSERT_EQ(bt1->close(),OK);
        ASSERT_EQ(bt2->close(),OK);

        ASSERT_EQ(bt1->remove(),OK);
        ASSERT_EQ(bt2->remove(),OK);
}


    TEST_F(pmemdsTest, BasicQueueTest) {
        pmemdsclient::priority_queue *pq = new pmemdsclient::priority_queue(testClient,pq1,1024*1024*8, 0UL);

        ASSERT_EQ(pq->create(PM_CREAT),OK);

        ASSERT_EQ(pq->insert(1UL,1UL),OK);
        ASSERT_EQ(pq->insert(2UL,3UL),OK);
        ASSERT_EQ(pq->insert(3UL,2UL),OK);
        unsigned long max;
        ASSERT_EQ(pq->get_max(max),OK);
        ASSERT_EQ(max,2UL);
        ASSERT_EQ(pq->remove(),FAILED); // currently a volatile data-structure present

    }

}
