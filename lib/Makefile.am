libwallyd_la_CFLAGS=$(CURL_CFLAGS) -D__WITH_MURMUR -std=gnu99 -g -I${buildroot}/include -I${buildroot}/include/duv -DWAIT_EV
libwallyd_la_SOURCES=jsmn.c buf.c json.c util.c parseJson.c plugins.c hashtable.h hashtable.c murmur.h murmur.c connect.c ui.c sys.c memfs.c miniz.c slog.c getrssmem.c
#duktape.c
libwallyd_la_LDFLAGS = -version-info 1:0:0 $(CURL_LIBS) -lduktape -lduv -fPIC
lib_LTLIBRARIES = libwallyd.la
