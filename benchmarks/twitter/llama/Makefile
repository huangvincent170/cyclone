include ../../Makefile.inc

LLAMA_HOME?=/home/pfernando/llama

# other options : -DLL_MEMORY_ONLY -DLL_PERSISTENCE
LLAMA_CXXFLAGS= -fopenmp -I. -I$(LLAMA_HOME)/llama/include -I$(LLAMA_HOME)/benchmark/benchmarks

LLAMA_LIBS= -lboost_system

all: local server client

server: llama_server
client: llama_async_client
local: llama_local

llama_local:llama_local.cpp
	$(CXX) $(CXXFLAGS) $(LLAMA_CXXFLAGS) -DLL_MEMORY_ONLY llama_local.cpp $(LLAMA_LIBS) -o $@

llama_server:llama_server.cpp
	$(CXX) $(CXXFLAGS) $(LLAMA_CXXFLAGS) llama_server.cpp $(LLAMA_LIBS) $(SERVER_LIBS) -o $@

llama_async_client:llama_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) llama_async_client.cpp $(CLIENT_LIBS) -o $@

.PHONY:clean

clean:
	rm -f llama_local llama_async_client llama_server
