set(WALLYD_SRC wallyd.c)
set(DWALLYD_SRC dwallyd.c duv.c schema.c dukcurl.c)
set(TEST_SRC drmtest.c)

add_executable(swallyd ${WALLYD_SRC})
add_executable(dwallyd ${DWALLYD_SRC})
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_executable(drmtest ${TEST_SRC})
  target_link_libraries(drmtest EGL GLESv2 gbm)
  install(TARGETS drmtest DESTINATION ${BIN_INSTALL_DIR})
endif()

target_link_libraries(swallyd ${LIBUV_LIBRARIES} ${LIBSDL_LIBRARIES} wallyd)
target_link_libraries(dwallyd ${LIBUV_LIBRARIES} ${LIBSDL_LIBRARIES} wallyd)

install(TARGETS swallyd dwallyd DESTINATION ${BIN_INSTALL_DIR})
