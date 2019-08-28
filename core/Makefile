all: libcyclone.a

CXXFLAGS = -O3 -fPIC -DBOOST_LOG_DYN_LINK
#CXXFLAGS = -O3 -fPIC -DBOOST_LOG_DYN_LINK -D__COMMUTE
#CXXFLAGS = -O1 -fno-omit-frame-pointer -g -fPIC -DBOOST_LOG_DYN_LINK

#Hardware Workaround
#CXXFLAGS += -DWORKAROUND0
#CXXFLAGS += -D__LATENCY_TRACER
CXXFLAGS += $(CPPFLAGS)
CXXFLAGS += -DDPDK_STACK -std=gnu++0x
RTE_SDK?=/home/pfernando/dpdk
CXXFLAGS += -march=native -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX -I../pmdk/src/include -I../raft/include -I${RTE_SDK}/x86_64-native-linuxapp-gcc/include


#Depending on boost version/OS distro
#BOOST_THREAD_LIB=-lboost_thread-mt
BOOST_THREAD_LIB=-lboost_thread


libcyclone.a: libcyclone.o dispatcher.o dispatch_client.o
	ar rcs $@ $^

libcyclone.o: cyclone.cpp libcyclone.hpp
	$(CXX) $(CXXFLAGS) cyclone.cpp -c -o $@

dispatcher.o: dispatcher.cpp libcyclone.hpp
	$(CXX) $(CXXFLAGS) dispatcher.cpp -c -o $@

dispatch_client.o: dispatch_client.cpp  libcyclone.hpp
	$(CXX) $(CXXFLAGS) dispatch_client.cpp -c -o $@

#flash_log.o: flash_log.cpp  libcyclone.hpp
#	$(CXX) $(CXXFLAGS) flash_log.cpp -c -o $@

.PHONY:clean install

install:libcyclone.a
	#	cp libcyclone.a /usr/lib
	#	cp libcyclone.hpp /usr/include

clean:
	rm -f libcyclone.o dispatcher.o dispatch_client.o libcyclone.a

