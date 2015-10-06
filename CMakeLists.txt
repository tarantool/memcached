cmake_minimum_required(VERSION 2.8)

include(cmake/build_libmemcached.cmake)
libmemcached_build()
include(cmake/build_memtier.cmake)
memtier_build()
