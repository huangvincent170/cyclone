/*
 * test cases emulating Noria (OSDI 2018) vote workload
 */

#include <string>
#include <pmemds.h>

#include "../include/pmemds-client.h"
#include "../include/pmemds-common.h"
#include "../include/tree/btree-client.h"
#include "../include/priority_queue/priority_queue-client.h"
#include "gtest/gtest.h"
#include "test_client.h"


namespace {

// The fixture for testing class Foo.
    class voteTest : public ::testing::Test {
    protected:
        const uint16_t art_st = 0;
        const uint16_t votes_st = 1;
        /* Code here will be called immediately after the constructor (right
           before each test). */
        void SetUp() override {

            const std::string pmem_path = "/dev/shm/pmemds";
            /* server side data-structure creation */
            pmemds::PMLib *pmLib = new pmemds::PMLib(pmem_path);

            testClient = new pmemdsclient::TestClient(pmLib,&request,&response);
            testClient->open("testApp");

            /* create art and vote structures */
            art = new pmemdsclient::BTreeEngine(testClient,art_st,100,0UL);
            art->create(PM_CREAT);
            votes = new pmemdsclient::priority_queue(testClient,votes_st,100,0UL);
            votes->create(PM_CREAT);
            /* put some articles */

            for(int i = 0; i < 20; i++){
                art->put(i, "article_" + std::to_string(i));
            }


            /* cast some votes */
            for(int i = 0; i < 20; i++){
                votes->insert(i,i);
            }

        }

        /* Code here will be called immediately after each test (right
           before the destructor). */
        void TearDown() override {


            art->remove();
            // our prio-queue is volatile still.
            //votes->remove();

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



    TEST_F(voteTest, VoteRTest) {
        unsigned long max;
        for(int i = 20; i > 0; i--){
            ASSERT_EQ(votes->get_max(max),OK);
        }
    }

    /* retrieve top K post from the vote db */
    /*Test_F(voteTest, VoteTopKTest){



    }*/

}
