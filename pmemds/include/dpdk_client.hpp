#ifndef PMEMDS_DPDK_CLIENT_HPP
#define PMEMDS_DPDK_CLIENT_HPP
#include "pmemds-client.h"

namespace pmemdsclient {

    class DPDKPMClient : public PMClient {
    public:
        DPDKPMClient(void *clnt);

        ~DPDKPMClient();

        int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask,int rpc_flags);

        int sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, int rpc_flags, void (*cb)(void *));

    private:
        void *dpdk_client;
        unsigned long request_id;
    };

}
#endif //PMEMDS_DPDK_CLIENT_HPP
