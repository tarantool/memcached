add_library(internalso SHARED
        "internal/memcached_constants.c"
        "internal/memcached_layer.c"
        "internal/memcached_network.c"
        "internal/memcached_persistent.c"
        "internal/memcached.c"
)

target_link_libraries(internalso small)

set_target_properties(internalso PROPERTIES PREFIX "" OUTPUT_NAME "internal")

# Install module
install(FILES memcached.lua DESTINATION ${TARANTOOL_LUADIR}/${PROJECT_NAME}/)
install(TARGETS  internalso LIBRARY DESTINATION ${TARANTOOL_LIBDIR}/${PROJECT_NAME}/)
