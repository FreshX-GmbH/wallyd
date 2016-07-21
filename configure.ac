
AC_PREREQ(2.69)

AC_INIT([Wallyd], [1.0], [support@freshx.de])
AC_CONFIG_SRCDIR([src])

AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIRS([m4])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_PROG_CC(gcc clang)
AC_PROG_CXX(g++)

AM_INIT_AUTOMAKE([-Wall foreign])
AM_PROG_AR
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
PKG_PROG_PKG_CONFIG

AC_SUBST(buildroot, `pwd`)
AC_SUBST(ac_hostname)


AC_ENABLE_SHARED

LT_PREREQ([2.2])
LT_INIT

dnl video
AC_ARG_ENABLE([video],
  [AS_HELP_STRING([--disable-video],
    [omit support for video output (libffmpeg required)])],
  [ VIDEO="NO" ], [VIDEO="YES"]
)

AC_ARG_ENABLE([sdl],
  [AS_HELP_STRING([--disable-sdl],
    [omit support for sdl / screen output])],
  [[disable_sdl="yes"],[enable_video="no"]],
)

AC_ARG_ENABLE([raspberry], 
  [AS_HELP_STRING([--enable-raspberry],
      [Build for Raspberry Pi 1/2])],
   [enable_raspberry="yes"]
)

AC_ARG_ENABLE([opencv], 
  [AS_HELP_STRING([--enable-opencv],
      [Build OpenCV based plugins])],
   [enable_opencv="yes"]
)

#AC_ARG_WITH([log4c], AS_HELP_STRING([--without-log4c], [do not use log4c]))

AS_IF([test "x$enable_raspberry" = "xyes"],
   [
    AC_CHECK_HEADER([bcm_host.h], [], [AC_MSG_WARN([Raspberry Pi 1/2 headers not found])])
    AC_CHECK_LIB([bcm_host], [bcm_host_init], [], [AC_MSG_WARN([Raspberry Pi 1/2 libs not found])])
    CFLAGS="${CFLAGS} -DRASPBERRY"
    LDFLAGS="${LDFLAGS} -lbcm_host"
])

AC_ARG_ENABLE([mali], 
  [AS_HELP_STRING([--enable-mali],
      [Build for Mali based GPUs - experimental])],
  [],
  [AS_IF([test "x$have_mali" = "xyes"],
   [enable_mali="yes"],
   [enable_mali="no"
])])

AS_IF([test "x$enable_mali" = "xyes"],
   [
    CFLAGS="${CFLAGS} -DMALI"
])

AC_ARG_ENABLE([directfb], 
  [AS_HELP_STRING([--enable-directfb],
      [Build for QEMU or DirectFN - experimental])],
  [],
  [AS_IF([test "x$have_directfb" = "xyes"],
   [enable_directfb="yes"],
   [enable_directfb="no"
])])

AS_IF([test "x$enable_dmalloc" != "xno"],
[   
   AC_CHECK_HEADERS([dmalloc.h],[
      CFLAGS="${CFLAGS} -DWALLY_DMALLOC"                  
   ])
])

AS_IF([test "x$enable_opencv" = "xyes"],
[     PKG_CHECK_MODULES(opencv, [opencv],
          [
            AC_SEARCH_LIBS(opencv)
            CFLAGS="${CFLAGS} ${opencv_CFLAGS} -DWALLY_OPENCV"
            LDFLAGS="${LDFLAGS} ${opencv_LIBS}"
            AC_CHECK_HEADERS(opencv/cv.h)
          ], 
          [
                AC_MSG_ERROR([Could not find opencv library.])
          ])
])

AS_IF([test "x$enable_directfb" = "xyes"],
[     PKG_CHECK_MODULES(directfb, [directfb fusion],
          [
            AC_SEARCH_LIBS(directfb.a fusion.a)
            CFLAGS="${CFLAGS} ${directfb_CFLAGS}"
            LDFLAGS="${LDFLAGS} ${directfb_LIBS}"
            AC_CHECK_HEADERS(directfb.h fusion.h)
          ], 
          [
                AC_MSG_ERROR([Could not find DirectFB library. Its needed for this build])
          ])
])
AS_IF([test "x$enable_video" != "xno"],
    [PKG_CHECK_MODULES(avformat, [libavformat],
     [
          CFLAGS="${CFLAGS} ${avformat_CFLAGS}"
          LDFLAGS="${LDFLAGS} ${avformat_LIBS}"
          AC_CHECK_HEADERS(libavformat/avformat.h ffmpeg/avformat.h)
     ],
     [
          AC_MSG_ERROR([Could not find AVFormat library])
     ])
    PKG_CHECK_MODULES(avcodec, [libavcodec],
     [
            CFLAGS="${CFLAGS} ${avcodec_CFLAGS}"
            LDFLAGS="${LDFLAGS} ${avcodec_LIBS}"
            AC_CHECK_HEADERS(libavcodec/avcodec.h ffmpeg/avcodec.h)
     ],
     [
            AC_MSG_ERROR([Could not find AVCodec library])
     ])
    
    PKG_CHECK_MODULES(avutil, [libavutil],
     [
            CFLAGS="${CFLAGS} ${avutil_CFLAGS}"
            LDFLAGS="${LDFLAGS} ${avutil_LIBS}"
            AC_CHECK_HEADERS(libavutil/avutil.h ffmpeg/avutil.h)
     ],
     [
            AC_MSG_ERROR([Could not find AVUtil library])
     ])
    
    PKG_CHECK_MODULES(swresample, [libswresample],
     [
            CFLAGS="${CFLAGS} ${swresample_CFLAGS}"
            LDFLAGS="${LDFLAGS} ${swresample_LIBS}"
            AC_CHECK_HEADERS(libswresample/swresample.h ffmpeg/swresample.h)
     ],
     [
            AC_MSG_ERROR([Could not find SWresample library])
     ])
    
    PKG_CHECK_MODULES(swscale, [libswscale],
     [
            CFLAGS="${CFLAGS} ${swscale_CFLAGS}"
            LDFLAGS="${LDFLAGS} ${swscale_LIBS}"
            AC_CHECK_HEADERS(libswscale/swscale.h ffmpeg/swscale.h)
     ],
     [
            AC_MSG_ERROR([Could not find SWScale library])
     ])
    CFLAGS="${CFLAGS} -DWALLY_VIDEO"
    ]
) 

PKG_CHECK_MODULES(uv, [libuv],
   [
          CFLAGS="${CFLAGS} ${uv_CFLAGS}"
          LDFLAGS="${LDFLAGS} ${uv_LIBS}"
          AC_CHECK_HEADERS(uv.h)
   ],
   [ AC_MSG_ERROR([Could not find uv library])]
)


# Checks for optional libraries.
#AS_IF([test "x$with_log4c" != "xno"], [
#    PKG_CHECK_MODULES([LOG4C], log4c, [
#          CFLAGS="${CFLAGS} ${LOG4C_CFLAGS}"
#          LDFLAGS="${LDFLAGS} ${LOG4C_LIBS}"])
#])

# Checks for libraries.
AC_CHECK_LIB([z], [compress])
#AC_SEARCH_LIBS([pthreads])
#[
#     AC_MSG_ERROR([unable to find pthreads])
#])

PKG_CHECK_MODULES([SDL2], [sdl2 SDL2_image SDL2_ttf], [have_sdl2=yes], [have_sdl2=no])
if test "x$have_sdl2" = xno; then
    AC_MSG_ERROR([SDL 2.0 libraries not found.])
fi
CFLAGS="${CFLAGS} ${SDL2_CFLAGS} -I../lib"
LDFLAGS="${LDFLAGS} ${SDL2_LIBS}"

AC_CHECK_LIB(SDL2_image, IMG_Init)
# Checks for header files.

AC_CHECK_LIB([SDL2_gfx], [rotozoomSurface])
LDFLAGS="${LDFLAGS} -lSDL2_gfx"

AC_CHECK_HEADERS(SDL2/SDL.h SDL2/SDL_image.h)

AC_CHECK_HEADERS(uv.h)

PKG_CHECK_MODULES([PNG], [libpng >= 1.5])

PKG_CHECK_MODULES(curl, [libcurl >= 7],
 [
        CFLAGS="${CFLAGS} ${curl_CFLAGS}"
        LDFLAGS="${LDFLAGS} ${curl_LIBS}"
        AC_CHECK_HEADERS(curl/curl.h)
 ],
 [
        AC_MSG_ERROR([Could not find curl library])
 ])


#see if ttf-font-dir option is enabled
#AC_ARG_ENABLE(ttf-font-dir,[AS_HELP_STRING([--enable-ttf-font-dir=DIR],[directory containing GNU FreeFont TTF fonts])],[sdlappfontdir="$enableval"],[sdlappfontdir=""])
#AM_CONDITIONAL([FONTDIR], [test "x$sdlappfontdir" != "x"])

# Checks for header files.
AC_HEADER_STDC

AC_CONFIG_HEADERS([config.h])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memset strcasecmp strdup strerror strncasecmp strstr])

AC_SUBST(build_os)
AC_SUBST(make, make)
AC_SUBST(mach, AUTO_MACH)
AC_SUBST(os, AUTO_OS)
AC_SUBST(debug, NOBT)
AC_SUBST(pthread, -pthread)

AC_ARG_ENABLE([debug], 
              AC_HELP_STRING([--enable-debug], 
              [enables backtrace debugging (default is NO)]),
              [debug=BT], [debug=NOBT])

AC_CANONICAL_HOST
case $host_os in
   darwin* )    CFLAGS="${CFLAGS} -DDARWIN";        echo OS : Darwin ;;
   linux*)      CFLAGS="${CFLAGS} -DLINUX";         echo OS : LINUX ;;
   *BSD*)       CFLAGS="${CFLAGS} -DBSD";           echo OS : BSD ;;
   *mingw*)     CFLAGS="${CFLAGS} -DWIN_MINGW";     echo OS : Windows MING ;;
   *cygwin*)    CFLAGS="${CFLAGS} -DCYGWIN";        echo OS : Windows CYGWIN ;;
   *) AC_MSG_ERROR([Your platform is not currently supported]) ;;
esac

case "${build_cpu}" in
    i[[34567]]86*) mach=I386  ;;
    armv6l*)       mach=armv6l;;
    armv7l*)       mach=armv7l ;;
    mips*)         mach=MIPS;;
    ia64*)         mach=IA64;;
    x86_64*)       mach=X86_64;;
esac

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 lib/Makefile
                 plugins/Makefile])

AC_OUTPUT
