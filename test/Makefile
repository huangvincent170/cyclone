#all: counter_server counter_driver counter_coordinator\
counter_loader counter_add_node counter_delete_node\
counter_coordinator_driver counter_driver_mt counter_driver_noop_mt
#all: counter_server counter_driver_noop_mt counter_delete_node counter_add_node counter_loader counter_driver_mt
all: server client

server: echo_server

#client: echo_client
client: echo_async_client

CXXFLAGS = -O3 -DBOOST_LOG_DYN_LINK
#CXXFLAGS = -O1 -g -fno-omit-frame-pointer -DBOOST_LOG_DYN_LINK
#PMEM_LIB?=../nvml/src/nondebug

# ${PMEM_LIB}/libpmemobj.a ${PMEM_LIB}/libpmem.a
LIBS = -lc ../core/libcyclone.a ../raft/libcraft.a -lboost_system -lboost_date_time -lpthread
#DPDK extras
RTE_SDK=/home/vhuang31/dpdk-stable-16.11.11

CXXFLAGS += -I../core -DDPDK_STACK
CXXFLAGS += -march=native -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX  -I${RTE_SDK}/x86_64-native-linuxapp-gcc/include

LIBS += -L${RTE_SDK}/x86_64-native-linuxapp-gcc/lib -Wl,--whole-archive -Wl,-lrte_distributor -Wl,-lrte_reorder -Wl,-lrte_kni -Wl,-lrte_pipeline -Wl,-lrte_table -Wl,-lrte_port -Wl,-lrte_timer -Wl,-lrte_hash -Wl,-lrte_jobstats -Wl,-lrte_lpm -Wl,-lrte_power -Wl,-lrte_acl -Wl,-lrte_meter -Wl,-lrte_sched -Wl,-lrte_vhost -Wl,-lm -Wl,-lrt -Wl,--start-group -Wl,-lrte_kvargs -Wl,-lrte_mbuf -Wl,-lrte_ip_frag -Wl,-lrte_ethdev -Wl,-lrte_cryptodev -Wl,-lrte_mempool -Wl,-lrte_ring -Wl,-lrte_eal -Wl,-lrte_cmdline -Wl,-lrte_cfgfile -Wl,-lrte_pmd_bond -Wl,-lrte_pmd_vmxnet3_uio  -Wl,-lrte_pmd_cxgbe -Wl,-lrte_pmd_enic -Wl,-lrte_pmd_i40e -Wl,-lrte_pmd_fm10k -Wl,-lrte_pmd_ixgbe -Wl,-lrte_pmd_e1000 -Wl,-lrte_pmd_ena -Wl,-lrte_pmd_ring -Wl,-lrte_pmd_af_packet -Wl,-lrte_pmd_null -Wl,-lrte_pmd_null_crypto -Wl,-lrte_pmd_vhost -Wl,-ldl -Wl,--end-group -Wl,--no-whole-archive -lpmem -lpmemobj

-CXXFLAGS += -I /usr/include -L /usr/lib
LIBS += -lucp -lucs

#BOOST_THREAD_LIB=-lboost_thread-mt
BOOST_THREAD_LIB=-lboost_thread

common.o:common.cpp
	$(CXX) $(CXXFLAGS) -c common.cpp -o $@

echo_server:echo_server.cpp
	$(CXX) $(CXXFLAGS) echo_server.cpp $(BOOST_THREAD_LIB) $(LIBS) -o $@

echo_client:echo_client.cpp 
	$(CXX) $(CXXFLAGS) echo_client.cpp $(BOOST_THREAD_LIB) $(LIBS) -o $@

echo_async_client:echo_async_client.cpp 
	$(CXX) $(CXXFLAGS) echo_async_client.cpp $(BOOST_THREAD_LIB) $(LIBS) -o $@

.PHONY:clean

clean:
	rm -f echo_server echo_client echo_async_client


