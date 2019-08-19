

include ../../Makefile.inc

server: pmemkv_server
client: pmemkv_async_client

#pmemkv
PMEMKV_HOME?=${BLIZZARD_HOME}../pmemkv
PMEMKV_CXXFLAGS=-std=c++11 -I${PMEMKV_HOME}/src
PMEMKV_LIBS=${PMEMKV_HOME}/bin/libpmemkv.a



pmemkv_server:pmemkv_server.cpp
	$(CXX) $(CXXFLAGS) $(PMEMKV_CXXFLAGS) pmemkv_server.cpp $(BOOST_THREAD_LIB) $(PMEMKV_LIBS) $(SERVER_LIBS) -o $@

pmemkv_client:pmemkv_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) pmemkv_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@

pmemkv_async_client:pmemkv_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) pmemkv_async_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@


.PHONY:clean

clean:
	rm -f \
pmemkv_client pmemkv_async_client pmemkv_server
