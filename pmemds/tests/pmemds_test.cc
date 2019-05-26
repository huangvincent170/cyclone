
#include <string>

#include "../src/pmemds-client.h"
#include "../src/pmemds-common.h"
#include "../src/tree/btree-client.h"
#include "../src/priority_queue/priority_queue-client.h"
#include "gtest/gtest.h"


namespace {

// The fixture for testing class Foo.
    class pmemdsTest : public ::testing::Test {
    protected:

        /* Code here will be called immediately after the constructor (right
           before each test). */
        void SetUp() override {

            /* server side data-structure creation */
            pmemds::PMLib *pmLib = new pmemds::PMLib();

            testClient = new pmemdsclient::TestClient(pmLib,&request,&response);
            testClient->open("testApp");
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
        pmemdsclient::BTreeEngine *bte = new pmemdsclient::BTreeEngine(testClient,"btree1",100,0UL);

        ASSERT_EQ(bte->create(PM_CREAT),0);
        //TODO: check state of request and response
        ASSERT_EQ(bte->put(1234UL,"test_1234"),0);
        std::string str = "test_1234";
        //ASSERT_STREQ(bte->get(1234UL),str);

        ASSERT_EQ(bte->remove(),0);

    }


    TEST_F(pmemdsTest, MultipleBTreeTest) {

    }


    TEST_F(pmemdsTest, BasicQueueTest) {

    }

}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
