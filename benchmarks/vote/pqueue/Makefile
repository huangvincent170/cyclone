
include ../../Makefile.inc

all: server client

server: pqueue_server
client: pqueue_async_client

#pmemds
#PMEMDS_HOME?=${BLIZZARD_HOME}../pmemds
PMEMDS_CXXFLAGS=-std=c++11 -I../../../pmemds/include -I../../../pmemds/include -I../../../pmdk/src/include -I../../../libpmemobj-cpp/include
PMEMDS_LIBS=../../../pmemds/bin/libpmemds.a
PMEMDSCLIENT_LIBS=../../../pmemds/bin/libpmemds-client.a



pqueue_server:pqueue_server.cpp
	$(CXX) $(CXXFLAGS) $(PMEMDS_CXXFLAGS) pqueue_server.cpp $(BOOST_THREAD_LIB) $(PMEMDS_LIBS) $(SERVER_LIBS) -ltbb -o $@

pqueue_client:pqueue_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) pqueue_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@

pqueue_async_client:pqueue_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) $(PMEMDS_CXXFLAGS) pqueue_async_client.cpp $(BOOST_THREAD_LIB) $(PMEMDSCLIENT_LIBS) $(CLIENT_LIBS) -o $@


.PHONY:clean

clean:
	rm -f \
pqueue_client pqueue_async_client pqueue_server
