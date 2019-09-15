
include ../../Makefile.inc

all: server client

server: adjvector_server
client: adjvector_async_client

#pmemds
PMEMDS_HOME?=../../../pmemds

PMEMDS_CXXFLAGS=-std=c++11 -I$(PMEMDS_HOME)/include
PMEMDS_LIBS=$(PMEMDS_HOME)/bin/libpmemds.a
PMEMDSCLIENT_LIBS=$(PMEMDS_HOME)/bin/libpmemds-client.a



adjvector_server:adjvector_server.cpp
	$(CXX) $(CXXFLAGS) $(PMEMDS_CXXFLAGS) adjvector_server.cpp $(BOOST_THREAD_LIB) $(PMEMDS_LIBS) $(SERVER_LIBS) -ltbb -o $@

adjvector_client:adjvector_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) adjvector_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@

adjvector_async_client:adjvector_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) $(PMEMDS_CXXFLAGS) adjvector_async_client.cpp $(BOOST_THREAD_LIB) $(PMEMDSCLIENT_LIBS) $(CLIENT_LIBS) -o $@


.PHONY:clean

clean:
	rm -f adjvector_client adjvector_async_client adjvector_server
