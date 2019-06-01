#ifndef __PMEMDS_CLIENT_H
#define __PMEMDS_CLIENT_H

#include <string>
#include "pmemds-common.h"
#include "pmemds_log.h"
#include "pmemds.h"


namespace pmemdsclient {

    /* abstraction to hide the real transport */
    class PMClient {
    public:
        PMClient(){};
        virtual ~PMClient(){};
        int open(const std::string &appname); // init transport
        int close();
        virtual int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask)=0;
        virtual int sendmsg_async(pm_rpc_t *msg, void (*cb)(void *, int,unsigned long))=0;
    protected:
        std::string appname;
    };


    class DPDKPMClient:public PMClient{
    public:
        DPDKPMClient(void *clnt);
        ~DPDKPMClient();

        int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask);
        int sendmsg_async(pm_rpc_t *msg, void (*cb)(void *, int,unsigned long));

    private:
        void *dpdk_client;
    };


    class TestClient:public PMClient{
    public:
        TestClient(pmemds::PMLib *pmLib ,pm_rpc_t *request, pm_rpc_t *response);
        ~TestClient(){};

        int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask);
        int sendmsg_async(pm_rpc_t *msg, void (*cb)(void *, int,unsigned long));

    private:
        pm_rpc_t *req, *res;
        pmemds::PMLib *pmLib;
    };


    class PMEngine {
    protected:
        PMClient *client;
        uint16_t ds_id;
        size_t size;
        unsigned long core_mask;

    public:

        virtual int create(uint8_t flags) = 0;
        virtual int close() = 0;
        virtual int remove()= 0;
    };



}
#endif