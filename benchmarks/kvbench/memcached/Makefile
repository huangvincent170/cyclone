CXX=g++
# memcached client that directly talks to memcached server.
all: client

client: memcached_local_client

LIBMEMCACHED_HOME?=/home/pfernando/rocksdb
MEMCACHED_LOCAL_CLIENT_CXXFLAGS=-std=c++11

MEMCACHED_LOCAL_CLIENT_LIBS += -lmemcached

memcached_local_client:memcached_local_client.cpp
	$(CXX) $(MEMCACHED_LOCAL_CLIENT_CXXFLAGS) memcached_local_client.cpp $(MEMCACHED_LOCAL_CLIENT_LIBS) -o $@


.PHONY:clean

clean:
	rm -f memcached_local_client
