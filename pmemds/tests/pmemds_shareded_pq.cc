

#include "../include/pmemds-client.h"
#include "../include/priority_queue/priority_queue-client.h"
#include "test_client.h"
#include "gtest/gtest.h"
#include "test_common.hpp"

namespace {

    uint8_t partition_callback(void *){
        return 0;
    }



// testing concurrent HashMap impl with our routing bits
    class pmemdsShardedPQTest : public ::testing::Test {
    protected:
        const uint16_t sharded_pq = 0;

        void SetUp() override {

            /* server side data-structure creation */
            pmLib = new pmemds::PMLib(pmem_path,1);

            shardedTestClient = new pmemdsclient::ShardedTestClient(pmLib, partition_callback);
            shardedTestClient->open("testApp"); // TODO: move to pmlib
        }

        /* Code here will be called immediately after each test (right
           before the destructor). */
        void TearDown() override {
            shardedTestClient->close();
            delete (shardedTestClient);
            delete (pmLib);
        }

        // Objects declared here can be used by all tests in the test case for Foo.
        pm_rpc_t request, response;
        pmemdsclient::ShardedTestClient *shardedTestClient;
        pmemds::PMLib *pmLib;

    };

    TEST_F(pmemdsShardedPQTest, BasicWTest) {
        pmemdsclient::PriorityQueueEngine *pq = new pmemdsclient::PriorityQueueEngine(shardedTestClient, sharded_pq, 1024 * 1024 * 8, 0UL);

        ASSERT_EQ(pq->create(PM_CREAT), OK);
        ASSERT_EQ(pq->insert(1234UL, 10UL), OK);
        ASSERT_EQ(pq->close(), OK);
        //ASSERT_EQ(pq->remove(), OK);
    }

}