#ifndef PMEMDS_TEST_CLIENT_H
#define PMEMDS_TEST_CLIENT_H

#include "pmemds.h"

namespace pmemdsclient {

    class TestClient : public PMClient {
    public:
        TestClient(pmemds::PMLib *pmLib, pm_rpc_t *request, pm_rpc_t *response);

        ~TestClient() {};

        int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask);

        int sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *));

    private:
        pm_rpc_t *req, *res;
        pmemds::PMLib *pmLib;
    };


    inline TestClient::TestClient(pmemds::PMLib *pmLib, pm_rpc_t *request,
                           pm_rpc_t *response) : req(request), res(response), pmLib(pmLib) {

    }


    inline int TestClient::sendmsg(pm_rpc_t *req, pm_rpc_t **response, unsigned long core_mask) {
        *response = new pm_rpc_t();
        pmLib->exec(req, *response);
        return 0; // no send errors
    }

    inline int TestClient::sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *)) {
        return 1;
    }


}


#endif //PMEMDS_TEST_CLIENT_H
