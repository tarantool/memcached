add_custom_target(test-memcached
        COMMAND ${PROJECT_SOURCE_DIR}/test/test-run.py binary)
add_dependencies(test-memcached internalso)

add_custom_target(test-memcached-capable
        COMMAND ${PROJECT_SOURCE_DIR}/test/test-run.py capable)
add_dependencies(test-memcached-capable libmemcached_copy)
