
include ../../Makefile.inc

all: server client

server: hashmap_server
client: hashmap_async_client

#pmemds
#PMEMDS_HOME?=${BLIZZARD_HOME}../pmemds
PMEMDS_CXXFLAGS=-std=c++11 -I../../../pmemds/include
PMEMDS_LIBS=../../../pmemds/bin/libpmemds.a
PMEMDSCLIENT_LIBS=../../../pmemds/bin/libpmemds-client.a



hashmap_server:hashmap_server.cpp
	$(CXX) $(CXXFLAGS) $(PMEMDS_CXXFLAGS) hashmap_server.cpp $(BOOST_THREAD_LIB) $(PMEMDS_LIBS) $(SERVER_LIBS) -o $@

hashmap_client:hashmap_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) hashmap_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@

hashmap_async_client:hashmap_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) $(PMEMDS_CXXFLAGS) hashmap_async_client.cpp $(BOOST_THREAD_LIB) $(PMEMDSCLIENT_LIBS) $(CLIENT_LIBS) -o $@


.PHONY:clean

clean:
	rm -f \
hashmap_client hashmap_async_client hashmap_server
