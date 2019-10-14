#include <sys/time.h>
#include <sys/resource.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <libgen.h>
#include <llama.h>
#include <benchmark.h>
#include <inttypes.h>

//#include "tools/property_stats.h"
//#include "tools/dump.h"
#include "clock.hpp"

std::string input_file = "/home/pradeep/blizzard-dev/cyclone/benchmarks/data/twitter/twitter_rv_15066953.net";
std::string database_directory = "llama_db";
#define MAX_GRAPH_NODES 10000

//stats
unsigned long tx_begin_time = 0UL;
unsigned long tx_cnt = 0UL;

typedef ll_mlcsr_ro_graph benchmarkable_graph_t;
#define benchmarkable_graph(g)  ((g).ro_graph())

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


/**
 * Load a part of the graph using the writable representation
 *
 * @param graph the writable graph
 * @param data_source the data source
 * @param loader_config the loader configuration
 * @param batch_size the batch size (the max number of edges to load)
 * @return true if the graph was loaded
 */
bool load_batch_via_writable_graph(ll_writable_graph& graph,
	ll_data_source& data_source, const ll_loader_config& loader_config,
	size_t batch_size) {


  bool loaded = data_source.pull(&graph, batch_size);
  if (!loaded) return false;
  graph.checkpoint(&loader_config);
  return true;
}




int main(int argc, char** argv)
{
  bool verbose = false;

  ll_loader_config loader_config;
  int num_threads = 4;

  double coin;
  double frac_read = 0.9;



  const char* first_input = input_file.c_str();
  const char* file_type = ll_file_extension(first_input);

  if (strcmp(ll_file_extension(input_file.c_str()), file_type)!=0) {
	fprintf(stderr, "Error: All imput files must have the same "
		"file extension.\n");
	return 1;
  }


  // Open the database
  ll_database database(database_directory.c_str());
  if (num_threads > 0) database.set_num_threads(num_threads);
  ll_writable_graph& graph = *database.graph();

  // Load the graph
  ll_file_loaders loaders;
  ll_file_loader* loader = loaders.loader_for(first_input);
  if (loader == NULL) {
	fprintf(stderr, "Error: Unsupported input file type\n");
	return 1;
  }

  ll_concat_data_source combined_data_source;
  ll_data_source* d = loader->create_data_source(input_file.c_str());
  combined_data_source.add(d);

  benchmarkable_graph_t& G = benchmarkable_graph(graph);
  ll_t_outdegree<benchmarkable_graph_t>* benchmark = new ll_t_outdegree<benchmarkable_graph_t>();

  int preload_batch= 100*1000;
  int writing_batch = 1000;
  //int streaming_window = 10;

  benchmark->initialize(&G);

  //pre-loading
  if (!load_batch_via_writable_graph(graph, combined_data_source,loader_config, preload_batch)){
	printf("error pre-loading\n");
	exit(-1);
  }

  printf("Starting real-run\n");
  for(int i =0; i < 11000; i++){
	coin = ((double)rand())/RAND_MAX;
	if(coin > frac_read){ // update
	  if (!load_batch_via_writable_graph(graph, combined_data_source,
			loader_config, writing_batch)) break;
	tx_cnt+=writing_batch;
	}else{
	  node_t node_id = rand() % MAX_GRAPH_NODES;
	  unsigned long od = benchmark->outdegree(node_id);
	  //printf("outdegree of node, %" PRId64 ": %ld \n",node_id, od);
	  //return_d = run_benchmark(G, benchmark);
	  tx_cnt++;
	}
	if (tx_cnt >= 5000)
	{
	  unsigned long total_elapsed_time = (rtc_clock::current_time() - tx_begin_time);
	  std::cout << "LOAD = "
		<< ((double)1000000 * (tx_cnt)) / total_elapsed_time
		<< " tx/sec " <<std::endl;
	  tx_cnt   = 0;
	  tx_begin_time = rtc_clock::current_time();
	}
  }
  benchmark->finalize();

  return 0;
}
