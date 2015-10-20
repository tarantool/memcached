cmake_minimum_required(VERSION 2.8)

include(cmake/BuildLibmemcached.cmake)
include(cmake/BuildMemtier.cmake)
include(cmake/FindTarantool.cmake)

libmemcached_build()
     memtier_build()

project(memcached C CXX) # uncomment for C++

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebugInfo)
endif()
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})

# Find Tarantool and Lua dependecies
set(TARANTOOL_FIND_REQUIRED ON)
# find_package(Tarantool)
include_directories(${TARANTOOL_INCLUDEDIR})

# Find other dependecies

# Set CFLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra")
# Set CXXFLAGS (uncomment for C++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")

# Build module
add_subdirectory(memcached)

# Add `make test`
if(POLICY CMP0037)
cmake_policy(SET CMP0037 OLD) # don't blame `test` target name
endif(POLICY CMP0037)
add_custom_target(test COMMAND ${PROJECT_SOURCE_DIR}/test/test-run.py)
