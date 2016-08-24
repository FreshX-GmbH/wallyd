project(libwallyd)

set(LIB_SRC jsmn.c buf.c json.c util.c
    parseJson.c plugins.c hashtable.c
    connect.c sys.c memfs.c murmur.c
    miniz.c slog.c getrssmem.c ui.c
)

add_library(libwallyd SHARED ${LIB_SRC})

target_link_libraries(libwallyd ${LIBUV_LIBS} ${LIBSDL_LIBRARIES} 
    ${LIBCURL_LIBRARIES} ${LIBSDLTTF_LIBRARIES} ${LIBSDLIMG_LIBRARIES} ${LIBDUV_LIBRARIES})

install(TARGETS libwallyd DESTINATION ${LIB_INSTALL_DIR})
