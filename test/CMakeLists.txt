add_custom_target(test-memcached
        COMMAND ${CMAKE_SOURCE_DIR}/test/test-run.py --force)
add_dependencies(test-memcached internalso)

add_custom_target(test-memcached-capable
        COMMAND ${CMAKE_SOURCE_DIR}/test/test-run.py --suite capable)

add_custom_target(test-memcached-bench
        COMMAND ${CMAKE_SOURCE_DIR}/test/test-run.py --force --long --suite bench)
add_dependencies(test-memcached-bench internalso memtier)
