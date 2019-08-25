

#include "../include/pmemds-client.h"
#include "../include/hashmap/hashmap-client.h"
#include "test_client.h"
#include "gtest/gtest.h"
#include "test_common.hpp"

namespace {

    uint8_t partition_callback(void *){
        return 0;
    }


// testing concurrent HashMap impl with our routing bits
    class pmemdsShardedHMTest : public ::testing::Test {
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

    TEST_F(pmemdsShardedHMTest, BasicWTest) {
        pmemdsclient::HashMapEngine *hm = new pmemdsclient::HashMapEngine(shardedTestClient, sharded_pq, 1024 * 1024 * 8, 0UL,1);

        ASSERT_EQ(hm->create(PM_CREAT), OK);
        ASSERT_EQ(hm->put(1234UL, "test_1234"), OK);
        ASSERT_EQ(hm->close(), OK);
        ASSERT_EQ(hm->remove(), OK);
    }

    TEST_F(pmemdsShardedHMTest, BasicRTest) {
        pmemdsclient::HashMapEngine *hm = new pmemdsclient::HashMapEngine(shardedTestClient, sharded_pq, 1024 * 1024 * 8, 0UL,1);

        ASSERT_EQ(hm->create(PM_CREAT), OK);
        ASSERT_EQ(hm->put(1234UL, "test_1234"), OK);
        ASSERT_EQ(hm->put(1345UL, "test_1345"), OK);
        ASSERT_EQ(hm->put(1456UL, "test_1456"), OK);
        ASSERT_EQ(hm->get(1345UL),"test_1345");
        ASSERT_EQ(hm->close(), OK);
        ASSERT_EQ(hm->remove(), OK);
    }
}