#ifndef _CYCLONE_UCP_
#define _CYCLONE_UCP_
#include <ucp/api/ucp.h>

#ifndef _CYCLONE_
#include "cyclone.hpp"
#endif

typedef struct cyclone_ucp_listener_context {
    volatile ucp_conn_request_h conn_request;
    ucp_listener_h              listener;
} cyclone_ucp_listener_context_t;

int cyclone_ucp_init_context(ucp_context_h *ucp_context, ucp_worker_h *ucp_worker);
ucs_status_t cyclone_ucp_init_listener(ucp_worker_h *ucp_worker, cyclone_ucp_listener_context_t *listener_ctx, ucp_listener_h *listener_p);
int cyclone_ucp_send(const char *ip, void *pkt, int sz);
int cyclone_ucp_recv(ucp_worker_h ucp_conn_worker, ucp_context_h ucp_context, cyclone_ucp_listener_context_t *ucp_listener_cxt, void *buf);
#endif