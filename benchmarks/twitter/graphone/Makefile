include ../../Makefile.inc

GRAPHONE_HOME?=/home/cyclone/graphone

# other options : -DLL_MEMORY_ONLY -DLL_PERSISTENCE
GRAPHONE_CXXFLAGS= -fopenmp -I. -I$(GRAPHONE_HOME)/graphone/include -I$(GRAPHONE_HOME)/benchmark/benchmarks

GRAPHONE_LIBS= -lboost_system libgraphone_server_lib.a libtest64.a libsrc64.a libonedata64.a -ltbb -fopenmp

all: server client

server: graphone_server
client: graphone_async_client
local: graphone_local

graphone_local:graphone_local.cpp
	$(CXX) $(CXXFLAGS) $(GRAPHONE_CXXFLAGS) -DLL_MEMORY_ONLY graphone_local.cpp $(GRAPHONE_LIBS) -o $@

graphone_server:graphone_server.cpp
	$(CXX) $(CXXFLAGS) $(GRAPHONE_CXXFLAGS) graphone_server.cpp $(GRAPHONE_LIBS) $(SERVER_LIBS) -o $@

graphone_async_client:graphone_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) graphone_async_client.cpp $(CLIENT_LIBS) -o $@

.PHONY:clean

clean:
	rm -f graphone_local graphone_async_client graphone_server
