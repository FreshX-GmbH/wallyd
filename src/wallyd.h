#ifndef WALLYD_H
#define WALLYD_H

#define _GNU_SOURCE
#include "autoversion.h"
#include "default.h"
#include "util.h"
#include "plugins.h"
#include "hashtable.h"
#include "ui.h"
#include <dlfcn.h>
#include <signal.h>
#include <dirent.h>
#include <uv.h>

#define BUFFERSIZE 8192

#define DEFAULT_THREAD_DELAY 2

//  0 = Auto detect
#define DEFAULT_WINDOW_WIDTH 0
#define DEFAULT_WINDOW_HEIGHT 0

// Global
extern FILE *fifo;
extern char __BUILD_DATE;
extern char __BUILD_NUMBER;

uv_fs_t openReq;
uv_fs_t readReq;
uv_fs_t closeReq;
#ifndef DWALLYD
uv_loop_t * loop;
#else
uv_loop_t loop;
#endif

char *startupScript = "/etc/wallyd.startup";

void onNewConnection(uv_stream_t *server, int status);
bool processCommand(char *cmd);
void onRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);
void allocBuffer(uv_handle_t* handle, size_t size, uv_buf_t* buf);

extern char *commandURL;

void *openLogfile(char *);
void initializeConfig(void);

pluginHandler *pluginsInit(void);
int pluginLoader(char *path);

bool registerClient(char *);
void url_init(void);
void setDebug(char *lvl);

int sendBroadcastPacket(void);
void ssdpDiscovery(int);
void *wallyClientThread(void *argPtr);
int sendWallyRegister();
int sendWallyLog(char *log);
bool getCommand(char *);
int cleanupPlugins(void);

char *initSDLThread(pluginHandler *);
void uvThread(void *p);
void readOptions(int argc, char **argv);
void processStartupScript(char *file);
void initSysPlugin(void *);
static void my_duk_fatal(duk_context *ctx, int code, const char *msg);
extern duk_ret_t duv_main(duk_context *ctx);
extern duk_ret_t dukopen_curl(duk_context *ctx);

#ifdef RASPBERRY
#include "bcm_host.h"
#endif

#endif
