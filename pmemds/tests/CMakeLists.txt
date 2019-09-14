configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif ()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 3.12)
    include_directories("${gtest_SOURCE_DIR}/include")
endif ()

include_directories(${CMAKE_SOURCE_DIR}/src/include
        ${CMAKE_BINARY_DIR}/bin)

set(TEST_FILES
        vote_test.cc
        priority_queue/pq_test.cc
        pmemds_test.cc
        pmemds_hashmap.cc
        test_main.cc)

# ---[ TEST SUITE
add_executable(test_suite ${TEST_FILES})

# ---[ by default shared libs gets linked. we force static libs
function(find_static_library LIB_NAME PATH OUT)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    find_library(
            FOUND_${LIB_NAME}_STATIC
            ${LIB_NAME}
            ${PATH}
    )
    if (FOUND_${LIB_NAME}_STATIC)
        get_filename_component(ABS_FILE ${FOUND_${LIB_NAME}_STATIC} ABSOLUTE)
    else()
        message(SEND_ERROR "Unable to find library ${LIB_NAME}")
    endif()
    set(${OUT} ${ABS_FILE} PARENT_SCOPE)
endfunction()

# -- find static versions of our libs. We don't want to add these paths to LD_LIBRARY_PATHS
find_static_library(pmem  ${PMDK_LIB} PMEM)
find_static_library(pmemobj ${PMDK_LIB} PMEMOBJ)


target_link_libraries(test_suite gtest_main pmemds-test pmemds  boost_system ${PMEMOBJ} ${PMEM} dl)
add_test(NAME TestSuite COMMAND test_suite)

# --[ Add "make check" target

set(CTEST_FLAGS "")
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_FLAGS} --verbose)