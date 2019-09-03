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


    inline int TestClient::sendmsg(pm_rpc_t *req, pm_rpc_t **response,unsigned long core_mask) {
        //*response = new pm_rpc_t();
        int resp_size;
        pmLib->exec(0,req, response, &resp_size);
        return resp_size;
    }

    inline int TestClient::sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *)) {
        return 1;
    }



    class ShardedTestClient : public PMClient {
    public:
        ShardedTestClient(pmemds::PMLib *pmLib, int (*partition_cb)(pm_rpc_t *request));

        ~ShardedTestClient() {};

        int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask);

        int sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *));

    private:
        int (*partition)(pm_rpc_t *request);
        pmemds::PMLib *pmLib;
    };


    inline ShardedTestClient::ShardedTestClient(pmemds::PMLib *pmLib, int (*partition_cb)(pm_rpc_t *request)){
        this->pmLib = pmLib;
        this->partition = partition_cb;
    }


    inline int ShardedTestClient::sendmsg(pm_rpc_t *req, pm_rpc_t **response, unsigned long core_mask) {
        //*response = new pm_rpc_t();
        int resp_size;
        uint8_t part = this->partition(req);
        pmLib->exec(part, req, response, &resp_size);
        return resp_size;
    }

    inline int ShardedTestClient::sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *)) {
        return 1;
    }

}

#endif //PMEMDS_TEST_CLIENT_H
