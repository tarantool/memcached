macro   (memtier_build)
    set(memtier_src ${CMAKE_CURRENT_SOURCE_DIR}/third_party/memtier)
    add_custom_target(memtier_cfg autoreconf -ivf
        COMMAND ./configure
        WORKING_DIRECTORY ${memtier_src}
    )
    add_custom_target(memtier_make
        COMMAND make
        DEPENDS memtier_cfg
        WORKING_DIRECTORY ${memtier_src}
    )
    add_custom_target(memtier_copy
        COMMAND ${CMAKE_COMMAND} -E copy ${memtier_src}/memtier_benchmark
                                 ${CMAKE_CURRENT_BINARY_DIR}/test/bench/memtier_benchmark
        DEPENDS memtier_make
    )
    add_custom_target(memtier DEPENDS memtier_copy)
endmacro(memtier_build)
