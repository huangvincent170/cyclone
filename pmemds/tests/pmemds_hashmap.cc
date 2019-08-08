
#include <string>

#include "../include/pmemds-client.h"
#include "../include/hashmap/hashmap-client.h"
#include "test_client.h"
#include "gtest/gtest.h"
#include "bench.h"


namespace {

// testing concurrent HashMap impl with our routing bits
    class pmemdsHashMapTest : public ::testing::Test {
    protected:
        const uint16_t hashmap1 = 0;

        /* Code here will be called immediately after the constructor (right
           before each test). */
        void SetUp() override {
            //const std::string pmem_path = "/dev/shm/pmemds_test";
            const std::string pmem_path = "/mnt/pmem1/pmemds_test";
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
/*
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

    TEST_F(pmemdsHashMapTest, WriteMoreHashMapTest) {
        //double coin;
        int keys = 1000;
        //float frac_read = 0.5;

        pmemdsclient::HashMapEngine *hm = new pmemdsclient::HashMapEngine(testClient, hashmap1, 1024 * 1024 * 8, 0UL);
        ASSERT_EQ(hm->create(PM_CREAT), OK);
        for(int i = 0; i < 10000; i++){
            //coin = ((double)rand()) / RAND_MAX;
            unsigned long key = rand() % keys;
            std::string str = "ts_" + std::to_string(key);
            ASSERT_EQ(hm->put(key, str.c_str()), OK);
        }
        ASSERT_EQ(hm->close(), OK);
        ASSERT_EQ(hm->remove(), OK);
    }



    TEST_F(pmemdsHashMapTest, RWMoreHashMapTest) {
        //double coin;
        int keys = 10000;
        //float frac_read = 0.5;

        pmemdsclient::HashMapEngine *hm = new pmemdsclient::HashMapEngine(testClient, hashmap1, 1024 * 1024 * 8, 0UL);
        ASSERT_EQ(hm->create(PM_CREAT), OK);

        for(int i = 0; i < keys; i++){
            //coin = ((double)rand()) / RAND_MAX;
            std::string str = "ts_" + std::to_string(i);
            ASSERT_EQ(hm->put(i, str.c_str()), OK);
        }

        for(int i = 0; i < 100000; i++){
            //coin = ((double)rand()) / RAND_MAX;
            unsigned long key = rand() % keys;
            std::string str = "ts_" + std::to_string(key);
            ASSERT_EQ(hm->put(key, str.c_str()), OK);
        }

        for(int i = 0; i < 100000; i++){
            //coin = ((double)rand()) / RAND_MAX;
            unsigned long key = rand() % keys;
            std::string str = "ts_" + std::to_string(key);
            ASSERT_STREQ(hm->get(key).c_str(), str.c_str());
        }


        ASSERT_EQ(hm->close(), OK);
        ASSERT_EQ(hm->remove(), OK);
    }*/



    static void hashmap_bench(void *engine, thread_state_t *th_state){
        pmemdsclient::HashMapEngine *hm = (pmemdsclient::HashMapEngine *) engine;
        int keys = 10000;
        for(int i = 0; i < keys*100; i++){
            //coin = ((double)rand()) / RAND_MAX;
            unsigned long key = th_state->rand.Next() % keys;
            std::string str = "ts_" + std::to_string(key);
            ASSERT_EQ(hm->put(key, str.c_str()), OK);
            th_state->stats.nops_++;
        }
    }


    TEST_F(pmemdsHashMapTest, OneConcurrentHashMapTest) {
        pmemdsclient::HashMapEngine *hm =
                new pmemdsclient::HashMapEngine(testClient, hashmap1, 1024 * 1024 * 8, 0UL);
        ASSERT_EQ(hm->create(PM_CREAT), OK);
        Benchmark *benchmark = new Benchmark();
        benchmark->run(1,(void*)hm ,hashmap_bench);
        ASSERT_EQ(hm->close(), OK);
        ASSERT_EQ(hm->remove(), OK);
    }

    TEST_F(pmemdsHashMapTest, TwoConcurrentHashMapTest) {
        pmemdsclient::HashMapEngine *hm =
                new pmemdsclient::HashMapEngine(testClient, hashmap1, 1024 * 1024 * 8, 0UL);
        ASSERT_EQ(hm->create(PM_CREAT), OK);
        Benchmark *benchmark = new Benchmark();
        benchmark->run(2,(void*)hm ,hashmap_bench);
        ASSERT_EQ(hm->close(), OK);
        ASSERT_EQ(hm->remove(), OK);
    }

    TEST_F(pmemdsHashMapTest, ThreeConcurrentHashMapTest) {
        pmemdsclient::HashMapEngine *hm =
                new pmemdsclient::HashMapEngine(testClient, hashmap1, 1024 * 1024 * 8, 0UL);
        ASSERT_EQ(hm->create(PM_CREAT), OK);
        Benchmark *benchmark = new Benchmark();
        benchmark->run(3,(void*)hm ,hashmap_bench);
        ASSERT_EQ(hm->close(), OK);
        ASSERT_EQ(hm->remove(), OK);
    }

}


