AM_CPPFLAGS=-I$(top_srcdir)/lib $(CFLAGS) -Wno-deprecated-declarations -Wno-unused-variable -D__WITH_MURMUR -g -I${buildroot}/include/duv -Wno-extra-tokens -std=c99 -I. 

lib_LTLIBRARIES = myPlugin.la sdlffvideo.la client.la gpio.la watchdog.la svg2image.la ssdp.la texture.la sdlimage.la sdlgui.la screen.la js.la 

myPlugin_la_SOURCES = myPlugin.c
myPlugin_la_LDFLAGS =-module -avoid-version -shared

texture_la_SOURCES = texture.c
texture_la_LDFLAGS =-module -avoid-version -shared

sdlimage_la_SOURCES = sdlimage.c
sdlimage_la_LDFLAGS =-module -avoid-version -shared

sdlgui_la_SOURCES = sdlgui.c
sdlgui_la_LDFLAGS =-module -avoid-version -shared

screen_la_SOURCES = screen.c
screen_la_LDFLAGS =-module -avoid-version -shared

js_la_SOURCES = js.c
js_la_LDFLAGS =-module -avoid-version -shared

ssdp_la_SOURCES = ssdp.c
ssdp_la_LDFLAGS =-module -avoid-version -shared

opencv_la_SOURCES = opencv.c
opencv_la_LDFLAGS =-module -avoid-version -shared

svg2image_la_SOURCES = svg2image.c
svg2image_la_LDFLAGS =-module -avoid-version -shared

watchdog_la_SOURCES = watchdog.c
watchdog_la_LDFLAGS =-module -avoid-version -shared

gpio_la_SOURCES = gpio.c
gpio_la_LDFLAGS =-module -avoid-version -shared

sdlffvideo_la_SOURCES = sdlffvideo.c ffplay/cmdutils.c ffplay/postprocess.c ffplay/ffplay.c
sdlffvideo_la_LDFLAGS = -module -avoid-version -shared ${swscale_LDFLAGS}
sdlffvideo_la_CFLAGS  = $(AVLIBS) -I./ffplay -I./ffplay/include -DWALLY_PLUGIN ${swscale_CFLAGS}

client_la_SOURCES = register.c persist.c client.c
client_la_LDFLAGS = -module -avoid-version -shared


