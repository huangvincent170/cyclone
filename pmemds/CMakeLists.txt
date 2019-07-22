cmake_minimum_required(VERSION 3.11)
project(pmemds)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "-O3 -fPIC -Wall")
set(CMAKE_CXX_FLAGS "-g -fPIC -Wall")
add_definitions(-DDPDK_STACK
   -march=native -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(RTE_CSDK /home/pfernando/dpdk)


#include_directories(../libpmemobj-cpp/include)
include_directories(../pmdk/src/include)
include_directories(${RTE_CSDK}/x86_64-native-linuxapp-gcc/include)
include_directories(include)
include_directories("../core")

#link dirs
#link_directories(/home/pradeep/blizzard/pmdk/src/nondebug)
link_directories(../../pmdk/src/debug)


set(SERVERLIB_SOURCE_FILES
        src/pmemds.cc
        src/tree/btree.cc
        src/hashmap/hashmap.cc
        src/priority_queue/priority_queue.cc)

set(CLIENTLIB_SOURCE_FILES
        src/pmemds-client.cc
        src/tree/btree-client.cc
        src/hashmap/hashmap-client.cc
        src/priority_queue/priority_queue-client.cc)

set(CLIENTLIB_TEST_SOURCE_FILES
        src/pmemds-client.cc
        src/tree/btree-client.cc
        src/hashmap/hashmap-client.cc
        src/priority_queue/priority_queue-client.cc)

add_library(pmemds STATIC ${SERVERLIB_SOURCE_FILES})

#add_library(pmemds-client STATIC ${CLIENTLIB_SOURCE_FILES})

add_library(pmemds-client STATIC ${CLIENTLIB_TEST_SOURCE_FILES})

# ---[ CTest
include(CTest)
add_subdirectory(tests)
