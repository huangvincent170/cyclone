
#BLIZZARD
BLIZZARD_HOME?=/home/pfernando/cyclone

CXXFLAGS = -O3 -DBOOST_LOG_DYN_LINK -DBOOST_THREAD_DYN_LINK
CXXFLAGS += $(CPPFLAGS)
#CXXFLAGS = -O1 -g -fno-omit-frame-pointer -DBOOST_LOG_DYN_LINK
PMEM_SLIB?=/home/pfernando/pmdk/src/nondebug
PMEM_CLIB?=../../../pmdk/src/nondebug

LIBS = -lc ../../../core/libcyclone.a ../../../raft/libcraft.a -lboost_system -lboost_date_time -laio

#DPDK extras
RTE_CSDK?=/home/pfernando/dpdk
RTE_SSDK?=/home/pfernando/dpdk

CXXFLAGS += -DDPDK_STACK
CXXFLAGS += -march=native -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX

SERVER_CXXFLAGS= ${CXXFLAGS} -I${RTE_SSDK}/x86_64-native-linuxapp-gcc/include
CLIENT_CXXFLAGS= ${CXXFLAGS} -I${RTE_CSDK}/x86_64-native-linuxapp-gcc/include

SERVER_LIBS= ${LIBS} -L${RTE_SSDK}/x86_64-native-linuxapp-gcc/lib
CLIENT_LIBS= ${LIBS} -L${RTE_CSDK}/x86_64-native-linuxapp-gcc/lib

LIBS2 = -Wl,--whole-archive -Wl,-lrte_distributor -Wl,-lrte_reorder -Wl,-lrte_kni -Wl,-lrte_pipeline -Wl,-lrte_table -Wl,-lrte_port -Wl,-lrte_timer -Wl,-lrte_hash -Wl,-lrte_jobstats -Wl,-lrte_lpm -Wl,-lrte_power -Wl,-lrte_acl -Wl,-lrte_meter -Wl,-lrte_sched -Wl,-lrte_vhost -Wl,-lm -Wl,--start-group -Wl,-lrte_kvargs -Wl,-lrte_mbuf -Wl,-lrte_ip_frag -Wl,-lrte_ethdev -Wl,-lrte_net -Wl,-lrte_cryptodev -Wl,-lrte_mempool -Wl,-lrte_ring -Wl,-lrte_eal -Wl,-lrte_cmdline -Wl,-lrte_cfgfile -Wl,-lrte_pmd_bond -Wl,-lrte_pmd_vmxnet3_uio -Wl,-lrte_pmd_virtio -Wl,-lrte_pmd_cxgbe -Wl,-lrte_pmd_enic -Wl,-lrte_pmd_i40e -Wl,-lrte_pmd_fm10k -Wl,-lrte_pmd_ixgbe -Wl,-lrte_pmd_e1000 -Wl,-lrte_pmd_ena -Wl,-lrte_pmd_ring -Wl,-lrte_pmd_af_packet -Wl,-lrte_pmd_null -Wl,-lrte_pmd_null_crypto -Wl,-lrte_pmd_vhost -Wl,-ldl -Wl,--end-group -Wl,--no-whole-archive

SERVER_LIBS += ${LIBS2}
CLIENT_LIBS += ${LIBS2}

SERVER_LIBS += ${PMEM_SLIB}/libpmemobj.a ${PMEM_SLIB}/libpmem.a -lndctl -ldaxctl -lpthread
CLIENT_LIBS += ${PMEM_CLIB}/libpmemobj.a ${PMEM_CLIB}/libpmem.a -lndctl -ldaxctl -lpthread

#BOOST_THREAD_LIB=-lboost_thread-mt
BOOST_THREAD_LIB=-lboost_thread




