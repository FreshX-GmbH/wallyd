#ifndef WALLY_PLUGIN_H
#define WALLY_PLUGIN_H

#define _GNU_SOURCE

#define WTX_SIZE 512

#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include "default.h"
#include "hashtable.h"
#include <duktape.h>

#define WALLY_ASSERT_PH_VALID(ph) do { \
                assert((ph) != NULL); \
        } while (0)

#define CALL_TYPE_PTR  0
#define CALL_TYPE_STR  1
#define CALL_TYPE_PS   2
#define CALL_TYPE_CTX  3
#define CALL_TYPE_WTX  4
#define CALL_TYPE_NULL 5
#define WALLY_EVENT (SDL_USEREVENT+5)
#define WALLY_CALL_PTR  (WALLY_EVENT+CALL_TYPE_PTR)
#define WALLY_CALL_STR  (WALLY_EVENT+CALL_TYPE_STR)
#define WALLY_CALL_PS   (WALLY_EVENT+CALL_TYPE_PS)
#define WALLY_CALL_WTX  (WALLY_EVENT+CALL_TYPE_WTX)
#define WALLY_CALL_CTX  (WALLY_EVENT+CALL_TYPE_CTX)
#define WALLY_CALL_NULL (WALLY_EVENT+CALL_TYPE_NULL)
#define WFUNC_THRD true
#define WFUNC_SYNC false

typedef struct wally_call_ctx{
    char *name[WTX_SIZE];
    char *param[WTX_SIZE];
    void *data[WTX_SIZE];
    int data_size[WTX_SIZE];
    int ret[WTX_SIZE];
    int type[WTX_SIZE];
    int elements;
    int transaction;
}  wally_call_ctx;

typedef struct{
    // SDL Image / Screen Stuff
    bool SDL;
    void *sdl_tid;
    void *window;
    void *glcontext;
    void *renderer;
    void **texturePrio;
    hash_table *baseTextures;
    void *tempTexture;
    void *guiTexture;
    bool autorender;
    int width;
    int height;
    bool broadcomInit;
    pthread_t videoThread;
    bool playVideo;
    bool disableAudio;
    bool disableVideo;
    bool disableVideoPQ;
    bool disableVideoDisplay;
    bool disableVideoAfterFinish;

    duk_context *ctx;

    // The global configuration and flags
    int loglevel;
    int logmask;
    bool daemonizing;
    bool logfile;
    FILE *logfileHandle;
    void *logCat;
    int sdldebug;

    // SSDP / cloud Client
    bool registered;
    char *basedir;
    char *location;
    char *uuid;
    bool ssdp;
    bool cloud;
    bool clientThreadRunning;
    bool ssdpRunning;
    int threadDelay;
    pthread_t wallyClientThread;
    pthread_t mainThread;
    
    bool VFSOpen;
    char *VFSName;
    long VFSSize;

    // Plugin system
    hash_table *callbacks;
    hash_table *thr_functions;
    hash_table *functionWaitConditions;
    hash_table *functions;
    hash_table *plugins;
    hash_table *fonts;
    hash_table *colors;
    hash_table *configMap;
    hash_table *configFlagsMap;

    void *funcMutex;
    bool pluginLoaderDone;
    int pluginCount;

    int uiAllCount;
    int uiOwnCount;
    int callCount;
    int textureCount;
    int eventDelay;
    int uiEventTimeout;
    int conditionTimeout;
    void *slg;

    bool transaction;
    wally_call_ctx *wtx;

} pluginHandler;

extern pluginHandler *ph;
typedef int (*wally_c_function)(char *parameter);
//typedef int (*wally_c_function)(wally_call_ctx *ctx);
typedef struct function_list_entry function_list_entry;
struct function_list_entry {
        const char *name;
        wally_c_function value;
        int threaded;
        int nargs;
};

int pluginLoader(char *);
bool exportSync(const char *, void *);
bool exportThreaded(const char *, void *);
bool callWithData(char *, void *, void *);
bool callWithString(char *, void *, char *);
bool call(char *, void *, char *);
bool callNonBlocking(char *, int *, void *);
int sendWallyCommand(char *, char *);
int cleanupPlugins(void);
bool callEx(char *, void *, void *,int ,bool );
void export_function_list(char *, const function_list_entry *);
void wally_put_function_list(pluginHandler *, function_list_entry *);
void wally_put_function(const char *name, int threaded, int (*f), int args);
bool callWtx(char *fstr, char *params);

#endif
