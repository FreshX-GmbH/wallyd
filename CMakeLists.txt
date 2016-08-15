CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(wallyd)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH on)

# Compile video support (requires libffmpeg)
set(WITH_VIDEO off)

# We compile agains seaduk ? Otherwise we compile agains dukluv
set(WITH_SEADUK on)

# We want to gently wait for events or poll agressivly (more performant)
set(WAIT_EV on)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -D__WITH_MURMUR -std=gnu99")

find_package(PkgConfig)
pkg_check_modules(LIBUV libuv>=1.7)
pkg_check_modules(LIBSDL sdl2>=2.0)
pkg_check_modules(LIBSDLTTF SDL2_ttf>=2.0)
pkg_check_modules(LIBSDLIMG SDL2_image>=2.0)
#pkg_check_modules(LIBSDLGFX SDL2_gfx>=1.0)
if(WITH_VIDEO STREQUAL on)
	set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWITH_VIDEO")
	pkg_check_modules(LIBSWRE libswresample)
	pkg_check_modules(LIBAVUTIL libavutil)
	pkg_check_modules(LIBAVCODEC libavcodec)
	pkg_check_modules(LIBAVFORMAT libavformat)
	pkg_check_modules(LIBSWSCALE libswscale)
endif()
pkg_check_modules(LIBCURL libcurl)

if(WITH_SEADUK STREQUAL on)
	set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWITH_SEADUK -std=gnu99")
	pkg_check_modules(LIBDUV seaduk)
else()
	pkg_check_modules(LIBDUV dukluv)
endif()

if(WAIT_EV STREQUAL on)
	set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWAIT_EV")
endif()

include_directories(${LIBUV_INCLUDE_DIRS} ${LIBSDL_INCLUDE_DIRS} ${LIBDUV_INCLUDE_DIRS} lib src)
link_directories(${LIBUV_LIBRARY_DIRS} ${LIBSDL_LIBRARY_DIRS})

# TODO : Build Versioning
#file (STRINGS "version" CMAKE_BUILD_NUMBER)
#execute_process(COMMAND echo \$\(\(${CMAKE_BUILD_NUMBER}+1\)\) OUTPUT_FILE version2)

string(TIMESTAMP CMAKE_BUILD_DATE "%Y-%m-%d")

set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE STRING "Directory where lib will install")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Directory where lib will install")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/etc" CACHE PATH "The directory the configs are installed in")
set(PLUGIN_INSTALL_DIR "${LIB_INSTALL_DIR}/wallyd" CACHE PATH "The directory the plugins are installed in")

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD") 
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DDARWIN")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DDARWIN")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux") 
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DLINUX")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DWINDOWS")
endif()

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(plugins)

set(PROJECT_DESCRIPTION "Wally Daemon")


set(SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc" CACHE PATH "System configuration directory")
set(PREFSDIR ${SYSCONFDIR}/${CMAKE_PROJECT_NAME}/wallyd.d)

configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/etc/wallyd.conf" "${CMAKE_CURRENT_SOURCE_DIR}/wallyd.conf")
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/lib/autoversion.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/lib/autoversion.h" )
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/wallyd.conf" DESTINATION ${SYSCONFDIR})
#install(FILES etc/wallyd.startup.js DESTINATION ${SYSCONFDIR})
install(DIRECTORY etc/wallyd.d DESTINATION ${SYSCONFDIR})

