#ifndef PMEMDS_ADJ_VECTOR_CLIENT_HPP
#define PMEMDS_ADJ_VECTOR_CLIENT_HPP


#include "../pmemds-client.h"
#include "../pmemds_log.h"



namespace pmemdsclient {

    class AdjVectorEngine : public PMEngine {

    public:
        AdjVectorEngine(PMClient *handle, const uint16_t ds_id, size_t size, unsigned long core_mask);
        AdjVectorEngine(PMClient *handle, const uint16_t ds_id, size_t size, unsigned long core_mask,
                        uint8_t npartitions);
        ~AdjVectorEngine();


        int create(uint8_t flags);
        int close();
        int remove();

        int create(uint8_t flags, void (*cb)(void *));
        int close(void (*cb)(void *));
        int remove(void (*cb)(void *));


        //sync member functions
        int add_node(unsigned long node_id);
        int remove_node(unsigned long node_id);
        int add_edge(unsigned long from_node, unsigned long to_node);
        int remove_edge(unsigned long from_node, unsigned long to_node);
        int incident_triangles(unsigned long node_id);
        int vertex_outdegree(unsigned long node_id);

        //async member functions
        int add_node(unsigned long node_id,void (*cb)(void *));
        int remove_node(unsigned long node_id,void (*cb)(void *));
        int add_edge(unsigned long from_node, unsigned long to_node,void (*cb)(void *));
        int remove_edge(unsigned long from_node, unsigned long to_node,void (*cb)(void *));
        int incident_triangles(unsigned long node_id,void (*cb)(void *));
        int vertex_outdegree(unsigned long node_id,void (*cb)(void *));

    private:
        std::string ds_name;
    };
}


#endif //PMEMDS_ADJ_VECTOR_CLIENT_HPP
