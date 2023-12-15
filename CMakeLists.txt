cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_STANDARD 99)

project(data_structures_c C)

set(MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MASTER_PROJECT ON)
endif ()
option(DATA_STRUCTURES_BUILD_TESTS "Build test suite" ${MASTER_PROJECT})

if (DATA_STRUCTURES_BUILD_TESTS)
    include(CTest)
    add_subdirectory(unittests)
endif ()

add_library(ds INTERFACE dsAllocator.h dsArray.h dsSet.h)
target_include_directories(ds INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
