bin_PROGRAMS = wallyd testaudio dwallyd wallyc sdl_test
#resample2 testvideo
testaudio_SOURCES = testaudio.c
testvideo_SOURCES = testvideo.c
ffplay_SOURCE = ffplay.c
resample_SOURCES = resample2.c
blend_SOURCES = blend.c
resample_LDFLAGS = -lswresample
testvideo_LDFLAGS = -lswresample -lavutil -lavcodec -lavformat -lswscale
ffplay_LDFLAGS = -lswresample -lavutil -lavcodec -lavformat -lswscale -lSDL -lavfilter -lavdevices cmdutils.o
ffplay_CFLAGS = -lswresample -lavutil -lavcodec -lavformat -lswscale cmdutils.c
wallyc_SOURCES= wallyc.cpp tcp.cpp
wallyd_SOURCES = wallyd.c
sdl_test_SOURCES = sdl_test.c
dwallyd_SOURCES = dwallyd.c duv.c schema.c dukcurl.c 
AM_LDFLAGS = -L${buildroot}/lib -lwallyd -L../lib -luv -lduv -lduktape -ldschema
#AM_LDFLAGS = -L${buildroot}/lib -lwallyd -lhashtable
AM_CFLAGS = -D__WITH_MURMUR -g -O0 -I${buildroot}/include -I${buildroot}/include/duv 
