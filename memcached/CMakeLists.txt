add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/memcached/internal/memcached_text.c
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${RAGEL} -G2 memcached/internal/memcached_text.rl
                     -o memcached/internal/memcached_text.c
    DEPENDS ${CMAKE_SOURCE_DIR}/memcached/internal/memcached_text.rl)

add_custom_target(generate_memcached_text_c DEPENDS
    ${CMAKE_SOURCE_DIR}/memcached/internal/memcached_text.c)

# do not randomly try to re-generate memcached_text.c
# after a fresh checkout/branch switch.
execute_process(COMMAND ${CMAKE_COMMAND} -E touch_nocreate
    ${CMAKE_SOURCE_DIR}/memcached/internal/memcached_text.c)

set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/memcached/internal/memcached_text.c
    PROPERTIES HEADER_FILE_ONLY true)

add_library(internalso SHARED
        "internal/memcached_constants.c"
        "internal/memcached_layer.c"
        "internal/memcached_network.c"
        "internal/memcached_persistent.c"
        "internal/memcached.c"
        "internal/memcached_text.c"
)

target_link_libraries(internalso small)

set_target_properties(internalso PROPERTIES PREFIX "" OUTPUT_NAME "internal")

message(STATUS "${TARANTOOL_LUADIR}:${TARANTOOL_LIBDIR}")

# Install module
install(FILES   init.lua   DESTINATION ${TARANTOOL_LUADIR}/${PROJECT_NAME}/)
install(TARGETS internalso LIBRARY DESTINATION ${TARANTOOL_LIBDIR}/${PROJECT_NAME}/)
