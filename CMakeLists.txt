cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Find Tarantool and Lua dependecies
set(TARANTOOL_FIND_REQUIRED ON)
find_package(Tarantool)
# include(cmake/FindTarantool.cmake)
include_directories(${TARANTOOL_INCLUDEDIR})

include(cmake/BuildLibmemcached.cmake)
include(cmake/BuildMemtier.cmake)

include_directories(${CMAKE_SOURCE_DIR}/third_party)

libmemcached_build()
     memtier_build()

project(memcached C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_program(RAGEL ragel)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/small EXCLUDE_FROM_ALL)

# Find other dependecies

# Set CFLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb3 -O0 -Werror")

# Enable GNU glibc extentions.
add_definitions("-D_GNU_SOURCE")

# Build module
add_subdirectory(memcached)

# Add `make test`
if(POLICY CMP0037)
    cmake_policy(SET CMP0037 OLD) # don't blame `test` target name
endif(POLICY CMP0037)
add_subdirectory(test)
