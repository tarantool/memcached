cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(memcached C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Find Tarantool and Lua dependencies
find_package(LuaCheck)
set(Tarantool_FIND_REQUIRED ON)
find_package(Tarantool)
find_package(CyrusSASL)
include_directories(${TARANTOOL_INCLUDE_DIRS})

# We should add -std=c99 before small library inclusion, because
# the library does not enforce a C dialect, but unable to be built
# with -std=gnu90 (which is default on GCC 4.8, which is shipped
# with Ubuntu Trusty).
#
# See https://github.com/tarantool/small/issues/25
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -std=c99")

find_program(GIT git)
if (EXISTS "${CMAKE_SOURCE_DIR}/.git" AND GIT)
    execute_process (COMMAND ${GIT} describe --long HEAD
        OUTPUT_VARIABLE TARANTOOL_GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    message(STATUS "Updating submodules")
    execute_process(COMMAND ${GIT} submodule update --init --recursive)
endif()

# Find MsgPuck library
option(ENABLE_BUNDLED_MSGPUCK "Enable building of the bundled MsgPuck" ON)
if (ENABLE_BUNDLED_MSGPUCK)
    find_package_message(MsgPuck "Using bundled MsgPuck" "${MSGPUCK_INCLUDE_DIRS}")
    set(MSGPUCK_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/third_party/msgpuck)
    add_library(msgpuck STATIC third_party/msgpuck/msgpuck.c)
    set(MSGPUCK_LIBRARY msgpuck)
else()
    set(MsgPuck_FIND_REQUIRED ON)
    find_package(MsgPuck)
endif()
include_directories(${MSGPUCK_INCLUDE_DIRS})

# Find small library
option(ENABLE_BUNDLED_SMALL "Enable building of the bundled Small" ON)
if (ENABLE_BUNDLED_SMALL)
    set(SMALL_EMBEDDED ON)
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/small EXCLUDE_FROM_ALL)
    set(SMALL_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/third_party/small ${PROJECT_SOURCE_DIR}/third_party/small/third_party)
    find_package_message(Small "Using bundled Small" "${SMALL_INCLUDE_DIRS}")
else()
    set(Small_FIND_REQUIRED ON)
    find_package(Small)
endif()
include_directories(${SMALL_INCLUDE_DIRS})

include(cmake/BuildLibmemcached.cmake)
include(cmake/BuildMemtier.cmake)
libmemcached_build()
     memtier_build()

include_directories(${CMAKE_SOURCE_DIR}/third_party)

find_program(RAGEL ragel)

# Find other dependencies

# Set CFLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb3 -O0 -Werror")

# Enable GNU glibc extentions.
add_definitions("-D_GNU_SOURCE")

if (APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined suppress -flat_namespace")
endif(APPLE)

# Build module
add_subdirectory(memcached)

add_subdirectory(test)

add_custom_target(luacheck
  COMMAND ${LUACHECK} ${PROJECT_SOURCE_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
