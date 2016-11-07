project(libwally)

set(LIB_SRC util.c
    plugins.c Hash_Table.c HNode.c
    sys.c memfs.c pqueue.c
    slog.c getrssmem.c ui.c
)

add_library(wally SHARED ${LIB_SRC})

target_link_libraries(wally ${LIBUV_LIBS} ${LIBSDL_LIBRARIES} 
    ${LIBSDLTTF_LIBRARIES} ${LIBSDLIMG_LIBRARIES} ${LIBDUV_LIBRARIES})

install(TARGETS wally DESTINATION ${LIB_INSTALL_DIR})
