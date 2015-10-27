cmake_minimum_required(VERSION 2.8)

include(cmake/BuildLibmemcached.cmake)
include(cmake/BuildMemtier.cmake)
include(cmake/FindTarantool.cmake)

include_directories(${CMAKE_SOURCE_DIR}/third_party)

libmemcached_build()
     memtier_build()

project(memcached C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})

# Find Tarantool and Lua dependecies
set(TARANTOOL_FIND_REQUIRED ON)
# find_package(Tarantool)
include_directories(${TARANTOOL_INCLUDEDIR})

# Find other dependecies

# Set CFLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -std=c11 -D_GNU_SOURCE")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fPIC -std=c11 -Wall -Wextra -D_GNU_SOURCE")

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/small)

# Build module
add_subdirectory(memcached)

# Add `make test`
if(POLICY CMP0037)
    cmake_policy(SET CMP0037 OLD) # don't blame `test` target name
endif(POLICY CMP0037)
add_subdirectory(test)
