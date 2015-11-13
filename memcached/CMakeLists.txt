add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/memcached/internal/proto_text_parser.c
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${RAGEL} -G2 memcached/internal/proto_text_parser.rl
                     -o memcached/internal/proto_text_parser.c
    DEPENDS ${CMAKE_SOURCE_DIR}/memcached/internal/proto_text_parser.rl)

add_custom_target(generate_proto_text_parser_c DEPENDS
    ${CMAKE_SOURCE_DIR}/memcached/internal/proto_text_parser.c)

# do not randomly try to re-generate proto_text_parser.c
# after a fresh checkout/branch switch.
execute_process(COMMAND ${CMAKE_COMMAND} -E touch_nocreate
    ${CMAKE_SOURCE_DIR}/memcached/internal/proto_text_parser.c)

# set_source_files_properties(
#     ${CMAKE_SOURCE_DIR}/memcached/internal/proto_text_parser.c
#     PROPERTIES HEADER_FILE_ONLY true)

add_library(internalso SHARED
        "internal/constants.c"
        "internal/utils.c"
        "internal/proto_binary.c"
        "internal/proto_text_parser.c"
        "internal/proto_text.c"
        "internal/network.c"
        "internal/memcached_layer.c"
        "internal/memcached.c"
)

set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM true)

add_dependencies(internalso generate_proto_text_parser_c)

target_link_libraries(internalso small)

set_target_properties(internalso PROPERTIES PREFIX "" OUTPUT_NAME "internal")

# Install module
install(FILES   init.lua   DESTINATION ${TARANTOOL_LUADIR}/${PROJECT_NAME}/)
install(TARGETS internalso LIBRARY DESTINATION ${TARANTOOL_LIBDIR}/${PROJECT_NAME}/)
