#include <unistd.h>
#include "vector/adj_vector.h"


#define DO_LOG 0
#define LOG(msg) if (DO_LOG) std::cout << "[adjvector] " << msg << "\n"

namespace pmemds{

    AdjVectorEngine::AdjVectorEngine(const string &path, size_t size) {
        if((access(path.c_str(),F_OK) !=0) && (size > 0)){
            LOG("Creating filesystem pool, path=" << path << ", size=" << to_string(size));
            pmpool = pmem::obj::pool<RootData>::create(path.c_str(), LAYOUT, size, S_IRWXU);
        }else {
            LOG("Opening pool, path=" << path);
            pmpool = pmem::obj::pool<RootData>::open(path.c_str(), LAYOUT);
        }
        recover();
        LOG("Opened ok");

    }

    AdjVectorEngine::~AdjVectorEngine(){
        LOG("Closing");
        pmpool.close();
        LOG("Closed ok");
    }


    void AdjVectorEngine::exec(uint8_t thread_id, uint16_t op_name, uint8_t ds_type, std::string ds_id, pm_rpc_t *req,
                               pm_rpc_t **resp_ptr, int *resp_size) {
        pm_rpc_t *resp = (pm_rpc_t *)SAFECALLOC(sizeof(pm_rpc_t));
        *resp_ptr = resp;
        *resp_size = sizeof(pm_rpc_t);

        switch (op_name){
            case ADD_NODE:

                break;
            case ADD_EDGE:
                add_edge(req->key,req->key2,resp);
                break;
            case VERTEX_DEGREE:
                vertex_outdegree(req->key,resp);
                break;
            default:
                LOG_ERROR("unknown operation");
                SET_STATUS(resp->meta,INVALID_OP);
        }
    }


    /* add en edge in to the graph structure. We create src/dest nodes if they do not exist */
    void AdjVectorEngine::add_edge(unsigned long from_node, unsigned long to_node, pm_rpc_t *resp) {

        if(from_node > MAX_NODES || to_node > MAX_NODES){
            LOG("graph is out of valid node range");
            exit(1);
        }
        // check if src and dest exists
        vector_t_ptr src_vpptr =  list_array[from_node];
        if(src_vpptr == nullptr){ //create source
          list_array[from_node]  = pmem::obj::make_persistent<vector_t>();
        }

        vector_t_ptr dest_vpptr =  list_array[to_node];
        if(dest_vpptr == nullptr){
          list_array[to_node]  = pmem::obj::make_persistent<vector_t>();
        }
        vector_t *pvector =  list_array[from_node].get();


        gnode_t *gnode = new gnode_t();
        gnode->node_id = to_node;
        pvector-> push_back(*gnode);

        SET_STATUS(resp->meta,OK);
    }


    void AdjVectorEngine::vertex_outdegree(unsigned long node_id, pm_rpc_t *resp) {
        if(node_id > MAX_NODES){
            LOG("graph is out of valid node range");
            exit(1);
        }
        vector_t  *pvector = list_array[node_id].get();
        int out_degree = pvector->size();
        SET_STATUS(resp->meta, OK);
        resp->key = out_degree;
    }


    void AdjVectorEngine::recover() {

        auto root_data = pmpool.root();
        if (!root_data->size) {
            //TODO : initialize during restart
        } else {
            pmem::obj::transaction::manual tx(pmpool);
            root_data->size = MAX_NODES;
            root_data->ptr_array = pmem::obj::make_persistent<vector_t_ptr[]>(MAX_NODES);
            pmem::obj::transaction::commit();
            list_array = root_data->ptr_array;

        }
    }


    void AdjVectorEngine::add_node(unsigned long node_id, pm_rpc_t *resp) {
        // TODO
    }

    void AdjVectorEngine::remove_node(unsigned long node_id, pm_rpc_t *resp) {
        //TODO
    }

    void AdjVectorEngine::remove_edge(unsigned long from_node, unsigned long to_node, pm_rpc_t *resp){

    }

    void AdjVectorEngine::incident_triangles(unsigned long node_id, pm_rpc_t *resp) {

    }
}