
#include <string>

#include "../include/pmemds-client.h"
#include "../include/vector/adj_vector-client.hpp"
#include "test_client.h"
#include "gtest/gtest.h"
#include "bench.h"


namespace {

// testing concurrent HashMap impl with our routing bits
    class pmemdsAdjVectorTest : public ::testing::Test {
    protected:
        const uint16_t adjvector = 0;

        /* Code here will be called immediately after the constructor (right
           before each test). */
        void SetUp() override {
            //const std::string pmem_path = "/dev/shm/pmemds_test";
            const std::string pmem_path = "/mnt/pmem1/pmemds_test";
            /* server side data-structure creation */
            pmLib = new pmemds::PMLib(pmem_path);

            testClient = new pmemdsclient::TestClient(pmLib);
            testClient->open("testApp"); // TODO: move to pmlib
        }

        /* Code here will be called immediately after each test (right
           before the destructor). */
        void TearDown() override {
            testClient->close();
            delete (testClient);
            delete (pmLib);
        }

        // Objects declared here can be used by all tests in the test case for Foo.
        pm_rpc_t request, response;
        pmemdsclient::TestClient *testClient;
        pmemds::PMLib *pmLib;


    };

    TEST_F(pmemdsAdjVectorTest, BasicAdjVectorRWTest) {
        pmemdsclient::AdjVectorEngine *graph = new pmemdsclient::AdjVectorEngine(testClient, adjvector, 1024 * 1024 * 8, 0UL);

        ASSERT_EQ(graph->create(PM_CREAT), OK);
        ASSERT_EQ(graph->add_edge(2UL,1UL), OK);
        ASSERT_EQ(graph->close(), OK);
        ASSERT_EQ(graph->remove(), OK);
    }

}


