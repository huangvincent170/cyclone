
#include <string>

#include "../include/pmemds-client.h"
#include "../include/vector/adjvector-client.h"
#include "test_common.hpp"
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
            setup_pmem(pmem_path);
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

    TEST_F(pmemdsAdjVectorTest, BasicAdjVectorWTest) {
        pmemdsclient::AdjVectorEngine *graph = new pmemdsclient::AdjVectorEngine(testClient, adjvector, 1024 * 1024 * 8, 0UL);

        ASSERT_EQ(graph->create(PM_CREAT), OK);
        ASSERT_EQ(graph->add_edge(2UL,1UL), OK);
        ASSERT_EQ(graph->close(), OK);
        ASSERT_EQ(graph->remove(), OK);
    }

    TEST_F(pmemdsAdjVectorTest, BasicAdjVectorRWTest) {
        pmemdsclient::AdjVectorEngine *graph = new pmemdsclient::AdjVectorEngine(testClient, adjvector, 1024 * 1024 * 8, 0UL);

        ASSERT_EQ(graph->create(PM_CREAT), OK);
        ASSERT_EQ(graph->add_edge(2UL,1UL), OK);
        ASSERT_EQ(graph->vertex_outdegree(2UL), 1);
        ASSERT_EQ(graph->add_edge(2UL,4UL), OK);
        ASSERT_EQ(graph->vertex_outdegree(2UL), 2);
        ASSERT_EQ(graph->add_edge(2UL,5UL), OK);
        ASSERT_EQ(graph->vertex_outdegree(2UL), 3);
        ASSERT_EQ(graph->close(), OK);
        ASSERT_EQ(graph->remove(), OK);
    }

    TEST_F(pmemdsAdjVectorTest, InvalidDataReadTest) {
        pmemdsclient::AdjVectorEngine *graph = new pmemdsclient::AdjVectorEngine(testClient, adjvector, 1024 * 1024 * 8, 0UL);

        ASSERT_EQ(graph->create(PM_CREAT), OK);
        ASSERT_EQ(graph->vertex_outdegree(2),0);
        ASSERT_EQ(graph->add_edge(1UL,2UL),OK);
        ASSERT_EQ(graph->vertex_outdegree(2),0);
        ASSERT_EQ(graph->add_edge(2UL,10UL),OK);
        ASSERT_EQ(graph->vertex_outdegree(2),1);
        ASSERT_EQ(graph->close(), OK);
        ASSERT_EQ(graph->remove(), OK);
    }

    TEST_F(pmemdsAdjVectorTest, InvalidDataWriteTest) {
        pmemdsclient::AdjVectorEngine *graph = new pmemdsclient::AdjVectorEngine(testClient, adjvector, 1024 * 1024 * 8, 0UL);
        ASSERT_EQ(graph->create(PM_CREAT), OK);
        ASSERT_EQ(graph->add_edge(15066954UL,1UL),FAILED);
        ASSERT_EQ(graph->close(), OK);
        ASSERT_EQ(graph->remove(), OK);
    }

    TEST_F(pmemdsAdjVectorTest, MaxNodeTest) {
        pmemdsclient::AdjVectorEngine *graph = new pmemdsclient::AdjVectorEngine(testClient, adjvector, 1024 * 1024 * 8, 0UL);

        ASSERT_EQ(graph->create(PM_CREAT), OK);
        ASSERT_EQ(graph->add_edge(15066953UL,1UL),OK);
        ASSERT_EQ(graph->vertex_outdegree(15066953UL),1);
        ASSERT_EQ(graph->close(), OK);
        ASSERT_EQ(graph->remove(), OK);
    }
}


