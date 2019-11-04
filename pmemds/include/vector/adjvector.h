#ifndef PMEMDS_ADJ_VECTOR_H
#define PMEMDS_ADJ_VECTOR_H

#include "pmemds.h"
#include "libpmemobj++/container/vector.hpp"


/*
 * Implments graph data-structure interface using persistent vector
 */

#define MAX_NODES 15066954



typedef struct gnode_st{ // graph node structure
    unsigned long node_id;
}gnode_t;

using vector_t = pmem::obj::vector<gnode_t>;
using vector_t_ptr = pmem::obj::persistent_ptr <vector_t>;

typedef struct vector_array_st{
    pmem::obj::persistent_ptr <vector_t_ptr> vector_array_ptr;  // array of persistent pointers

    void init(bool hard){
        //TODO: initial and restart init
    }
}vector_array_t;



namespace pmemds {
    class AdjVectorEngine : public PMEngine {
    public:
        AdjVectorEngine(const string &path, size_t size);          // default constructor
        ~AdjVectorEngine();                                        // default destructor
        const string ENGINE = "graph_engine";

        void *engine(uint8_t thread_id) {
            return NULL;
        }

        void exec(uint8_t thread_id, uint16_t op_name,
                  uint8_t ds_type, std::string ds_id, pm_rpc_t *req, pm_rpc_t **resp_ptr, int *resp_size);

        void add_node(unsigned long node_id, pm_rpc_t *resp);

        void remove_node(unsigned long node_id, pm_rpc_t *resp);

        void add_edge(unsigned long from_node, unsigned long to_node, pm_rpc_t *resp);

        void remove_edge(unsigned long from_node, unsigned long to_node, pm_rpc_t *resp);

        void incident_triangles(unsigned long node_id, pm_rpc_t *resp);

        void vertex_outdegree(unsigned long node_id, pm_rpc_t *resp);

    private:
        using gnode_t_ptr = pmem::obj::persistent_ptr<gnode_t>;
        using vector_t = pmem::obj::vector<gnode_t_ptr>;
        using vector_t_ptr = pmem::obj::persistent_ptr <vector_t>;
        struct RootData {
            pmem::obj::p<unsigned long> size;
            pmem::obj::persistent_ptr <vector_t_ptr[]> ptr_array;
        };

        using pool_t = pmem::obj::pool<RootData>;

        void recover();

        pool_t pmpool;
        pmem::obj::persistent_ptr<vector_t_ptr[]> list_array; // array of vectors
    };
}
#endif //PMEMDS_ADJ_VECTOR_H
