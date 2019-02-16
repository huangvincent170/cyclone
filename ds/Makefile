prefix=/usr/local

all: clean configure staticlib

reset:
	rm -rf /dev/shm/pmemkv /tmp/pmemkv

clean: reset
	rm -rf 3rdparty libpmemkv.so pmemkv_bench pmemkv_example pmemkv_stress pmemkv_test
	rm -rf ./bin googletest-*.zip
	rm -rf pmemkv_bench

configure:
	mkdir -p ./bin
	cd ./bin && cmake .. -DCMAKE_BUILD_TYPE=Release

staticlib:
	cd ./bin && make pmemkv

install:
	cp ./bin/libpmemkv.so $(prefix)/lib
	cp ./src/pmemkv.h $(prefix)/include/libpmemkv.h

uninstall:
	rm -rf $(prefix)/lib/libpmemkv.so
	rm -rf $(prefix)/include/libpmemkv.h

test: configure reset
	cd ./bin && make pmemkv_test
	PMEM_IS_PMEM_FORCE=1 ./bin/pmemkv_test
	rm -rf /dev/shm/pmemkv /tmp/pmemkv

bench: reset
	  g++ ./bench/db_bench.cc ./bench/port/port_posix.cc ./bench/util/env.cc ./bench/util/env_posix.cc ./bench/util/histogram.cc ./bench/util/logging.cc ./bench/util/status.cc -o pmemkv_bench /usr/local/lib/libpmemkv.so -I/usr/local/include -I./bench/include -I./bench -I./bench/util -O2 -std=c++11 -DOS_LINUX -fno-builtin-memcmp -march=native -ldl -lpthread -DNDEBUG
		  PMEM_IS_PMEM_FORCE=1 ./pmemkv_bench --db=/dev/shm/pmemkv --db_size_in_gb=1 --histogram=1
			  $(MAKE) reset


