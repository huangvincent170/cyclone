#ifndef _GRAPH_COMMON_
#define _GRAPH_COMMON_

uint64_t g_query_outdegree(uint64_t v_id);
void g_add_vertex(uint64_t vertex_id);
int g_add_edge(uint64_t source, uint64_t destination);
void g_dump();
void g_waitfor_archive();
int init_and_create_graph();

#endif
