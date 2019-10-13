#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/write_batch.h>

#include "../core/libcyclone.hpp"
#include "../core/logging.hpp"
#include "../core/clock.hpp"
#include "llama.hpp"

typedef ll_mlcsr_ro_graph benchmarkable_graph_t;
#define benchmarkable_graph(g)  ((g).ro_graph())

std::string input_file = "/home/pradeep/blizzard-dev/cyclone/benchmarks/data/twitter/twitter_rv_15066953.net";
std::string database_directory = "/mnt/pmem1/llama_db";
int preload_batch= 100*1000;

/* batching related structure */
typedef struct edge_st{
	unsigned long from;
	unsigned long to;
}edge_t;
const int LLAMA_INSGEST_BATCH_SIZE = 1000;
// edge_t edge_buffer[LLAMA_INGEST_BATCH_SIZE];

int batch_counter = 0;

ll_loader_config loader_config;
int num_threads = 4;

ll_writable_graph& graph;
ll_file_loaders loaders;
ll_file_loader* loader;
ll_concat_data_source combined_data_source;
ll_data_source* d;
benchmarkable_graph_t& G;
ll_t_outdegree<benchmarkable_graph_t>* benchmark;




template <class Graph>
class ll_t_outdegree : public ll_benchmark<Graph> {
	FILE* _out;
	public:

	ll_t_outdegree() : ll_benchmark<Graph>("") {
		_out = stdout;
	}

	virtual ~ll_t_outdegree(void) {
	}

	unsigned long outdegree(node_t nodeId){
		Graph& G = *this->_graph;
		return G.out_degree(nodeId);
	}

	virtual double run(void){
		// making compiler happy
		return NAN;
	}

};

bool load_batch_via_writable_graph(ll_writable_graph& graph,
		ll_data_source& data_source, const ll_loader_config& loader_config,
		size_t batch_size) {


	bool loaded = data_source.pull(&graph, batch_size);
	if (!loaded) return false;
	graph.checkpoint(&loader_config);
	return true;
}


void callback(const unsigned char *data,
		const int len,
		rpc_cookie_t *cookie, unsigned long *pmdk_state)
{
	cookie->ret_value  = malloc(sizeof(llama_res_t));
	cookie->ret_size   = sizeof(llama_res_t);
	llama_res_t *res = (llama_res_t *) cookie->ret_value;

	llama_req_t *req = (llama_req_t *)data;

	if(req->op == OP_ADD_EDGE){
		batch_counter++;
		if(batch_counter % LLAMA_INGEST_BATCH_SIZE){
			if (!load_batch_via_writable_graph(graph, combined_data_source,
						loader_config, LLAMA_INGEST_BATCH_SIZE)) break;
			batch_counter = 0;
		}
		res->state = 1UL;
	}else if(req->op == OP_OUT_DEGREE){
		res->outdegree = benchmark->outdegree(node_id);
		res->state = 1UL; // we hard code the return status for now
	}
}

int commute_callback(unsigned long cmask1, void *op1, unsigned long cmask2, void *op2)
{
	return 0; // not used
}

void gc(rpc_cookie_t *cookie)
{
	free(cookie->ret_value);
}

rpc_callbacks_t rpc_callbacks =  {
	callback,
	gc,
	commute_callback
};

/* opening and pre-loading the db */
void open_llama(){
	// Open the database
	ll_database database(database_directory.c_str());
	database.set_num_threads(num_threads);
	graph = *database.graph();

	// Load the graph
	loader = loaders.loader_for(first_input);
	if (loader == NULL) {
		fprintf(stderr, "Error: Unsupported input file type\n");
		return 1;
	}

	d = loader->create_data_source(input_file.c_str());
	combined_data_source.add(d);

	G = benchmarkable_graph(graph);
	benchmark = new ll_t_outdegree<benchmarkable_graph_t>();
	benchmark->initialize(&G);

	//pre-loading
	if (!load_batch_via_writable_graph(graph, combined_data_source,loader_config, preload_batch)){
		printf("error pre-loading\n");
		exit(-1);
	}
}


void preload_llama(){

}
void close_llama(){

}

int main(int argc, char *argv[])
{
	if(argc != 7) {
		printf("Usage1: %s replica_id replica_mc clients cluster_config quorum_config ports\n", argv[0]);
		exit(-1);
	}

	int server_id = atoi(argv[1]);
	cyclone_network_init(argv[4],
			atoi(argv[6]),
			atoi(argv[2]),
			atoi(argv[6]) + num_queues*num_quorums + executor_threads);

	init_llama();

	dispatcher_start(argv[4],
			argv[5],
			&rpc_callbacks,
			server_id,
			atoi(argv[2]),
			atoi(argv[3]));
}


