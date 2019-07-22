#ifndef __PMEMDS_CLIENT_H
#define __PMEMDS_CLIENT_H

#include <string>
#include "pmemds-common.h"
#include "pmemds_log.h"


void async_callback(void *args, int code, unsigned long msg_latency);


namespace pmemdsclient {

    /* abstraction to hide the real transport */
    class PMClient {
    public:
        PMClient(){};
        virtual ~PMClient(){};

        int open(const std::string &appname); // init transport
        int close();

        int open(const std::string &appname,  void (*cb)(void *));
        int close(void (*cb)(void *));


        virtual int sendmsg(pm_rpc_t *msg, pm_rpc_t **response, unsigned long core_mask)=0;
        virtual int sendmsg_async(pm_rpc_t *msg, unsigned long core_mask, void (*cb)(void *))=0;
    protected:
        std::string appname;
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

        virtual int create(uint8_t flags, void (*cb)(void *)) { return -1; };

        virtual int close(void (*cb)(void *)) { return -1; };

        virtual int remove(void (*cb)(void *)) { return -1; };
    };

}
#endif
