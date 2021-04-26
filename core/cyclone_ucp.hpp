#ifndef _CYCLONE_UCP_
#define _CYCLONE_UCP_
#include <ucp/api/ucp.h>
// #include <rte_mbuf.h>
#include "cyclone.hpp"
typedef struct ucp_listener_context {
    volatile ucp_conn_request_h conn_request;
    ucp_listener_h              listener;
} ucp_listener_context_t;

int init_context(ucp_context_h *ucp_context, ucp_worker_h *ucp_worker);
ucs_status_t init_listener(ucp_worker_h *ucp_worker, ucp_listener_context_t *listener_ctx, ucp_listener_h *listener_p);
int send_client_ucp(rpc_t *pkt, int sz);
int client_recv_data(ucp_worker_h ucp_conn_worker, ucp_context_h ucp_context, ucp_listener_context_t *ucp_listener_cxt, rpc_t *buf);
#endif