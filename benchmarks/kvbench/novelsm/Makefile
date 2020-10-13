
include ../../Makefile.inc

all: server client

server: novelsm_server novelsm_loader
client: novelsm_async_client

ROCKS_HOME?=/home/cyclone/lsm_nvm
# ROCKS_CXXFLAGS=-std=c++11 -I${ROCKS_HOME}/include/ -L${ROCKS_HOME} -DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX  -DOS_LINUX -fno-builtin-memcmp -DROCKSDB_FALLOCATE_PRESENT -DSNAPPY -DGFLAGS=gflags -DZLIB -DBZIP2 -DZSTD -DNUMA -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_BACKTRACE -march=native  "-DROCKSDB_JEMALLOC"
ROCKS_CXXFLAGS=-std=c++11 -I${ROCKS_HOME}/include/ -L${ROCKS_HOME}/out-static -fno-builtin-memcmp -fpermissive -pthread -DOS_LINUX -DLEVELDB_PLATFORM_POSIX -DMULTILEVEL_IMMUTABLE -DUSE_OFFSETS -DENABLE_RECOVERY -D_ENABLE_PREDICTION
# ROCKS_LIBS += ${ROCKS_HOME}/librocksdb.a ${ROCKS_HOME}/libbz2.a ${ROCKS_HOME}/libz.a ${ROCKS_HOME}/libsnappy.a -lsnappy -lgflags -lz -lbz2 -lzstd -lpthread
ROCKS_LIBS += ${ROCKS_HOME}/out-static/libleveldb.a -lnuma -ltcmalloc -lpthread

# PLATFORM_LDFLAGS=-pthread
# PLATFORM_LIBS=-lnuma -ltcmalloc
# PLATFORM_CCFLAGS= -fno-builtin-memcmp -fpermissive -pthread -DOS_LINUX -DLEVELDB_PLATFORM_POSIX -DMULTILEVEL_IMMUTABLE -DUSE_OFFSETS -DENABLE_RECOVERY -D_ENABLE_PREDICTION
# PLATFORM_CXXFLAGS=-std=c++0x -fno-builtin-memcmp -fpermissive -pthread -DOS_LINUX -DLEVELDB_PLATFORM_POSIX -DMULTILEVEL_IMMUTABLE -DUSE_OFFSETS -DENABLE_RECOVERY -D_ENABLE_PREDICTION
# PLATFORM_SHARED_CFLAGS=-fPIC


novelsm_server:rocksdb_server.cpp
	$(CXX) $(CXXFLAGS) $(ROCKS_CXXFLAGS) rocksdb_server.cpp $(BOOST_THREAD_LIB) $(ROCKS_LIBS) $(SERVER_LIBS) -o $@

rocksdb_checkpoint:rocksdb_checkpoint.cpp
	$(CXX) $(CXXFLAGS) $(ROCKS_CXXFLAGS) rocksdb_checkpoint.cpp $(ROCKS_LIBS) $(SERVER_LIBS) -o $@

novelsm_loader:rocksdb_loader.cpp
	$(CXX) $(CXXFLAGS) $(ROCKS_CXXFLAGS) rocksdb_loader.cpp $(ROCKS_LIBS) $(SERVER_LIBS) -o $@


novelsm_async_client:rocksdb_async_client.cpp
	$(CXX) $(CLIENT_CXXFLAGS) rocksdb_async_client.cpp $(BOOST_THREAD_LIB) $(CLIENT_LIBS) -o $@


.PHONY:clean

clean:
	rm -f \
novelsm_async_client novelsm_server rocksdb_checkpoint novelsm_loader
