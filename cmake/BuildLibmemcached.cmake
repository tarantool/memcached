macro   (libmemcached_build)
    set(libmemcached_src ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libmemcached)
    add_custom_target(libmemcached_cfg ./bootstrap.sh autoreconf
        COMMAND ./configure --enable-jobserver=no --enable-memaslap
                --enable-static --enable-shared=off
        WORKING_DIRECTORY ${libmemcached_src}
    )
    add_custom_target(libmemcached_make
        COMMAND make libmemcached/csl/parser.h clients/memcapable
                        clients/memslap clients/memaslap
        DEPENDS libmemcached_cfg
        WORKING_DIRECTORY ${libmemcached_src}
    )
    add_custom_target(libmemcached_copy
        COMMAND ${CMAKE_COMMAND} -E copy ${libmemcached_src}/clients/memcapable
                                 ${CMAKE_CURRENT_BINARY_DIR}/test/capable/memcapable
        COMMAND ${CMAKE_COMMAND} -E copy ${libmemcached_src}/clients/memaslap
                                 ${CMAKE_CURRENT_BINARY_DIR}/test/bench/memaslap
        COMMAND ${CMAKE_COMMAND} -E copy ${libmemcached_src}/clients/memslap
                                 ${CMAKE_CURRENT_BINARY_DIR}/test/bench/memslap
        DEPENDS libmemcached_make
    )
    add_custom_target(libmemcached DEPENDS libmemcached_copy)
endmacro(libmemcached_build)
