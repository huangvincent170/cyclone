
include ../../Makefile.inc

all: server client

server: rocksdb_server rocksdb_checkpoint rocksdb_loader
client: rocksdb_async_client

ROCKS_HOME?=/home/pfernando/rocksdb
ROCKS_CXXFLAGS=-std=c++11 -I${ROCKS_HOME}/include/ -L${ROCKS_HOME} -DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX  -DOS_LINUX -fno-builtin-memcmp -DROCKSDB_FALLOCATE_PRESENT -DSNAPPY -DGFLAGS=gflags -DZLIB -DBZIP2 -DZSTD -DNUMA -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_BACKTRACE -march=native  "-DROCKSDB_JEMALLOC" 
ROCKS_LIBS += ${ROCKS_HOME}/librocksdb.a ${ROCKS_HOME}/libbz2.a ${ROCKS_HOME}/libz.a ${ROCKS_HOME}/libsnappy.a -lsnappy -lgflags -lz -lbz2 -lzstd -lpthread


rocksdb_server:rocksdb_server.cpp
	$(CXX) $(CXXFLAGS) $(ROCKS_CXXFLAGS) rocksdb_server.cpp $(BOOST_THREAD_LIB) $(ROCKS_LIBS) $(SERVER_LIBS) -o $@

rocksdb_checkpoint:rocksdb_checkpoint.cpp
	$(CXX) $(CXXFLAGS) $(ROCKS_CXXFLAGS) rocksdb_checkpoint.cpp $(BOOST_THREAD_LIB) $(ROCKS_LIBS) $(SERVER_LIBS) -o $@

rocksdb_loader:rocksdb_loader.cpp
	$(CXX) $(CXXFLAGS) $(ROCKS_CXXFLAGS) rocksdb_loader.cpp $(BOOST_THREAD_LIB) $(ROCKS_LIBS) $(SERVER_LIBS) -o $@


rocksdb_async_client:rocksdb_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) rocksdb_async_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@


.PHONY:clean

clean:
	rm -f \
rocksdb_async_client rocksdb_server rocksdb_checkpoint rocksdb_loader
