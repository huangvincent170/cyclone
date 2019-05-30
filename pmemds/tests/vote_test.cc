/*
 * test cases emulating Noria (OSDI 2018) vote workload
 */

#include <string>

#include "../include/pmemds-client.h"
#include "../include/pmemds-common.h"
#include "../include/tree/btree-client.h"
#include "../include/priority_queue/priority_queue-client.h"
#include "gtest/gtest.h"


namespace {

// The fixture for testing class Foo.
    class voteTest : public ::testing::Test {
    protected:

        /* Code here will be called immediately after the constructor (right
           before each test). */
        void SetUp() override {

            /* server side data-structure creation */
            pmemds::PMLib *pmLib = new pmemds::PMLib();

            testClient = new pmemdsclient::TestClient(pmLib,&request,&response);
            testClient->open("testApp");

            /* create art and vote structures */
            art = new pmemdsclient::BTreeEngine(testClient,"art",100,0UL);
            art->create(PM_CREAT);
            votes = new pmemdsclient::priority_queue(testClient,"votes",100,0UL);
            votes->create(PM_CREAT);
            /* put some articles */


            /* cast some votes */


        }

        /* Code here will be called immediately after each test (right
           before the destructor). */
        void TearDown() override {


            art->remove();
            votes->remove();

            testClient->close();
            delete(testClient);
            delete(pmLib);
        }

        // Objects declared here can be used by all tests in the test case for Foo.
        pm_rpc_t request,response;
        pmemdsclient::TestClient *testClient;
        pmemds::PMLib *pmLib;
        pmemdsclient::BTreeEngine *art;
        pmemdsclient::priority_queue *votes;


    };

    TEST_F(voteTest, VoteWTest) {
        pmemdsclient::BTreeEngine *bte = new pmemdsclient::BTreeEngine(testClient,"btree1",100,0UL);

        ASSERT_EQ(bte->create(PM_CREAT),0);
        //TODO: check state of request and response
        ASSERT_EQ(bte->put(1234UL,"test_1234"),0);
        std::string str = "test_1234";
        //ASSERT_STREQ(bte->get(1234UL),str);

        ASSERT_EQ(bte->remove(),0);

    }

    TEST_F(voteTest, VoteRTest) {
        pmemdsclient::BTreeEngine *bte = new pmemdsclient::BTreeEngine(testClient,"btree1",100,0UL);

        ASSERT_EQ(bte->create(PM_CREAT),0);
        //TODO: check state of request and response
        ASSERT_EQ(bte->put(1234UL,"test_1234"),0);
        std::string str = "test_1234";
        //ASSERT_STREQ(bte->get(1234UL),str);

        ASSERT_EQ(bte->remove(),0);

    }

    /* retrieve top K post from the vote db */
    Test_F(voteTest, VoteTopKTest){



    }

}
