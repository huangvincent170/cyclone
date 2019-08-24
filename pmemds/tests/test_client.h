#ifndef PMEMDS_TEST_CLIENT_H
#define PMEMDS_TEST_CLIENT_H

#include "pmemds.h"

namespace pmemdsclient {

    class TestClient : public PMClient {
    public:
        TestClient(pmemds::PMLib *pmLib);

        ~TestClient() {};

        int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask);

        int sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *));

    private:
        pmemds::PMLib *pmLib;
    };


    inline TestClient::TestClient(pmemds::PMLib *pmLib):pmLib(pmLib) {

    }


    inline int TestClient::sendmsg(pm_rpc_t *req, pm_rpc_t **response, unsigned long core_mask) {
        *response = new pm_rpc_t();
        pmLib->exec(0,req, *response);
        return 0; // no send errors
    }

    inline int TestClient::sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *)) {
        return 1;
    }



    class ShardedTestClient : public PMClient {
    public:
        ShardedTestClient(pmemds::PMLib *pmLib, uint8_t (*partition_cb)(void *request));

        ~ShardedTestClient() {};

        int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask);

        int sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *));

    private:
        uint8_t (*partition)(void *request);
        pmemds::PMLib *pmLib;
    };


    inline ShardedTestClient::ShardedTestClient(pmemds::PMLib *pmLib, uint8_t (*partition_cb)(void *request)){
        this->pmLib = pmLib;
        this->partition = partition_cb;
    }


    inline int ShardedTestClient::sendmsg(pm_rpc_t *req, pm_rpc_t **response, unsigned long core_mask) {
        *response = new pm_rpc_t();
        uint8_t part = this->partition((void *)req);
        pmLib->exec(part, req, *response);
        return 0; // no send errors
    }

    inline int ShardedTestClient::sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *)) {
        return 1;
    }

}

#endif //PMEMDS_TEST_CLIENT_H
