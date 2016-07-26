CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(wallyd)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH on)

find_package(PkgConfig)
pkg_check_modules(LIBUV libuv>=1.7)
pkg_check_modules(LIBSDL libsdl2>=2.0)
pkg_check_modules(LIBSWRE libswresample)
pkg_check_modules(LIBAVUTIL libavutil)
pkg_check_modules(LIBAVCODEC libavcodec)
pkg_check_modules(LIBAVFORMAT libavformat)
pkg_check_modules(LIBSWSCALE libswscale)

#ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(lib)
#ADD_SUBDIRECTORY(plugins)

ADD_EXECUTABLE(wallyd ${WALLYD_SRC})
ADD_EXECUTABLE(dwallyd ${DWALLYD_SRC})

ADD_LIBRARY(libwallyd shared ${LIB_SRC})
SET(WALLYD_SRC src/wallyd.c)
SET(DWALLYD_SRC src/dwallyd.c src/duv.c src/schema.c src/dukcurl.c)
SET(LIB_SRC lib/jsmn.c lib/buf.c lib/json.c lib/util.c 
    lib/parseJson.c lib/plugins.c lib/hashtable.c lib/murmur.c 
    lib/connect.c lib/ui.c lib/sys.c lib/memfs.c
    lib/miniz.c lib/slog.c lib/getrssmem.c
)

include_directories(${LIBUV_INCLUDE_DIRS} ${LIBSDL_INCLUDE_DIRS})
link_directories(${LIBUV_LIBRARY_DIRS} ${LIBSDL_LIBRARY_DIRS} )

