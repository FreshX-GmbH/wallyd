cmake_minimum_required(VERSION 2.8.1)

if (APPLE)
    set(SOEXT "dylib")
else()
    set(SOEXT "so")
endif()

string(REPLACE ";" " -l" TEMP_DUV_LIBS "${LIBDUV_LIBRARIES}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -llibwallyd -L../lib -l${TEMP_DUV_LIBS}")

#add_library(myPlugin MODULE myPlugin.c)
add_library(texture MODULE texture.c)
add_library(sdlimage MODULE sdlimage.c)
add_library(sdlgui MODULE sdlgui.c)
add_library(screen MODULE screen.c)
add_library(js MODULE js.c)
#add_library(ssdp MODULE ssdp.c)
#add_library(opencv MODULE opencv.c)
add_library(svg2image MODULE svg2image.c)
#add_library(watchdog MODULE watchdog.c)
#add_library(gpio MODULE gpio.c)
#add_library(client MODULE register.c persist.c client.c)

if(WITH_VIDEO STREQUAL on)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Iplugins/ffplay -I./ffplay/include -DWALLY_PLUGIN ${LIBSWSCALE_CFLAGS}")
	add_library(sdlffvideo MODULE sdlffvideo.c ffplay/cmdutils.c ffplay/postprocess.c ffplay/ffplay.c)
	target_link_libraries(sdlffvideo ${LIBSWSCALE_LIBRARIES} ${LIBSWRE_LIBRARIES} 
	    ${LIBAVUTIL_LIBRARIES} ${LIBAVCODEC_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBSDL_LIBRARIES})
	install(TARGETS sdlffvideo DESTINATION ${LIB_INSTALL_DIR}/wallyd)
endif()

target_link_libraries(sdlgui ${LIBSDL_LIBRARIES} ${LIBSDLTTF_LIBRARIES} ${LIBSDLIMG_LIBRARIES} ${LIBDUV_LIBRARIES})
target_link_libraries(svg2image ${LIBSDL_LIBRARIES} ${LIBDUV_LIBRARIES})
target_link_libraries(sdlimage ${LIBSDL_LIBRARIES} ${LIBSDLTTF_LIBRARIES} ${LIBSDLIMG_LIBRARIES} ${LIBDUV_LIBRARIES} SDL2_gfx)

install(TARGETS texture sdlimage sdlgui screen js svg2image DESTINATION ${LIB_INSTALL_DIR}/wallyd)
