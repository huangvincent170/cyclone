cmake_minimum_required(VERSION 3.5)
project(pmemds)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "-O3 -fPIC -Wall")
set(CMAKE_CXX_FLAGS "-g -fPIC -Wall")


set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(/home/pradeep/blizzard/libpmemobj-cpp/include)
include_directories(/home/pradeep/blizzard/pmdk/src/include)
include_directories(include)
include_directories("../core")

#link dirs
#link_directories(/home/pradeep/blizzard/pmdk/src/nondebug)
link_directories(/home/pradeep/blizzard/pmdk/src/debug)


set(SERVERLIB_SOURCE_FILES
        src/pmemds.cc
        #src/tree/btree.h
        src/tree/btree.cc
        #src/hashmap/hashmap.h
        src/hashmap/hashmap.cc
        #src/priority_queue/priority_queue.h
        src/priority_queue/priority_queue.cc)

set(CLIENTLIB_SOURCE_FILES
        #src/pmemds-client.h
        src/pmemds-client.cc
        #src/tree/btree-client.h
        src/tree/btree-client.cc
        #src/priority_queue/priority_queue-client.h
        src/priority_queue/priority_queue-client.cc)


add_library(pmemds STATIC ${SERVERLIB_SOURCE_FILES})

add_library(pmemds-client STATIC ${CLIENTLIB_SOURCE_FILES})

# ---[ CTest
include(CTest)
add_subdirectory(tests)
