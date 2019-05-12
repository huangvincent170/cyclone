#ifndef __PMEMDS_CLIENT_H
#define __PMEMDS_CLIENT_H

#include <string>
#include "pmemds-common.h"
#include "pmemds_log.h"

namespace pmemdsclient {

    /* abstraction to hide the real transport */
    class PMClient {
    public:
        PMClient(void *clnt);
        ~PMClient();
        int open(const std::string &appname); // init transport
        int close();
        virtual int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask)=0;
        virtual int sendmsg_async(pm_rpc_t *msg, void (*cb)(void *, int,unsigned long))=0;
    protected:
        void *dpdk_client;
        std::string appname;
    };


    class DPDKPMClient:public PMClient{
    public:
        DPDKPMClient(void *clnt);
        ~DPDKPMClient();

        int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask);
        int sendmsg_async(pm_rpc_t *msg, void (*cb)(void *, int,unsigned long));
    };

    class PMEngine {
    public:
        PMEngine();
        ~PMEngine();
        uint16_t ds_id;
    };
}
#endif