
#include <string>

#include "../include/pmemds-client.h"
#include "../include/hashmap/hashmap-client.h"
#include "test_client.h"
#include "gtest/gtest.h"


namespace {

// testing concurrent HashMap impl with our routing bits
    class pmemdsHashMapTest : public ::testing::Test {
    protected:
        const uint16_t hashmap1 = 0;

        /* Code here will be called immediately after the constructor (right
           before each test). */
        void SetUp() override {
            const std::string pmem_path = "/dev/shm/pmemds";
            /* server side data-structure creation */
            pmLib = new pmemds::PMLib(pmem_path);

            testClient = new pmemdsclient::TestClient(pmLib, &request, &response);
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

    TEST_F(pmemdsHashMapTest, BasicHashMapRWTest) {
        pmemdsclient::HashMapEngine *hm = new pmemdsclient::HashMapEngine(testClient, hashmap1, 1024 * 1024 * 8, 0UL);

        ASSERT_EQ(hm->create(PM_CREAT), OK);
        //TODO: check state of request and response
        ASSERT_EQ(hm->put(1234UL, "test_1234"), OK);
        //std::string str = "test_1234";
        ASSERT_STREQ(hm->get(1234UL).c_str(), "test_1234");
        ASSERT_EQ(hm->close(), OK);
        ASSERT_EQ(hm->remove(), OK);
    }
}


