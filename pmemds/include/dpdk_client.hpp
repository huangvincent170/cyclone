#ifndef PMEMDS_DPDK_CLIENT_HPP
#define PMEMDS_DPDK_CLIENT_HPP


namespace pmdsclient {

    class DPDKPMClient : public PMClient {
    public:
        DPDKPMClient(void *clnt);

        ~DPDKPMClient();

        int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask);

        int sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *));

    private:
        void *dpdk_client;
        unsigned long request_id;
    };

}
#endif //PMEMDS_DPDK_CLIENT_HPP
