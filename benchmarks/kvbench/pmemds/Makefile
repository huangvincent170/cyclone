
include ../../Makefile.inc

all: server client

server: pmemds_server
client: pmemds_async_client

#pmemds
#PMEMDS_HOME?=${BLIZZARD_HOME}../pmemds
PMEMDS_CXXFLAGS=-std=c++11 -I../../../pmemds/include
PMEMDS_LIBS=../../../pmemds/bin/libpmemds.a
PMEMDSCLIENT_LIBS=../../../pmemds/bin/libpmemds-client.a



pmemds_server:pmemds_server.cpp
	$(CXX) $(CXXFLAGS) $(PMEMDS_CXXFLAGS) pmemds_server.cpp $(BOOST_THREAD_LIB) $(PMEMDS_LIBS) $(SERVER_LIBS) -o $@

pmemds_client:pmemds_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) pmemds_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@

pmemds_async_client:pmemds_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) $(PMEMDS_CXXFLAGS) pmemds_async_client.cpp $(BOOST_THREAD_LIB) $(PMEMDSCLIENT_LIBS) $(CLIENT_LIBS) -o $@


.PHONY:clean

clean:
	rm -f \
pmemds_client pmemds_async_client pmemds_server
