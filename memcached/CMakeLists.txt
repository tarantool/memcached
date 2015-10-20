add_library(imemcached SHARED
            memcached_constants.cc
            memcached_layer.cc
            memcached_network.cc
            memcached.cc
)
set_target_properties(imemcached PROPERTIES PREFIX "" OUTPUT_NAME "imemcached")

# Install module
install(FILES memcached.lua DESTINATION ${TARANTOOL_LUADIR}/${PROJECT_NAME}/)
install(TARGETS imemcached LIBRARY DESTINATION ${TARANTOOL_LIBDIR}/${PROJECT_NAME}/)
