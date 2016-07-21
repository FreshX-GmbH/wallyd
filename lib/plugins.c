#include "util.h"
#include "dlfcn.h"
#include <dirent.h>
#include <SDL_events.h>
#include <SDL_mutex.h>

#define SDLWAITTIMEOUT 2000

pluginHandler *ph;
//
//  Paramtyp 0 : pointer
//           1 : string
//           2 : PointerStruct *PS
//           3 : DukContext *ctx

bool callEx(char *funcNameTmp, void *ret, void *paramsTmp, int paramType,bool waitThread){
    if(!funcNameTmp){
        slog(LVL_QUIET,ERROR,"Invalid function name");
        return false;
    }
    void *params = NULL;
    char *funcName = strdup(funcNameTmp);
    ph->callCount++;   
    void *(*thr_func)(void *) = ht_get_simple(ph->thr_functions,funcName);
    if(thr_func){
        SDL_Event event;
        SDL_zero(event);
        switch(paramType) {
            case CALL_TYPE_PTR:
                if(paramsTmp) {
                    params = paramsTmp;
                    slog(LVL_ALL,DEBUG,"call( %s(<binary>) )",funcName);
                    event.type = WALLY_CALL_PTR;
                } else { 
                    slog(LVL_ALL,DEBUG,"call( %s(NULL) )",funcName);
                    event.type = WALLY_CALL_NULL;
                }
                break;
            case CALL_TYPE_STR:
                if(paramsTmp){
                    //params = strdup(paramsTmp);
                    params = paramsTmp;
                    slog(LVL_ALL,DEBUG,"call( %s(%s) )",funcName,params);
                    event.type = WALLY_CALL_STR;
                } else {
                    event.type = WALLY_CALL_NULL;
                }
                break;
            case CALL_TYPE_PS:
                // TODO
                // params = paramsTmp;
                slog(LVL_ALL,DEBUG,"call( %s(<PS *>) )",funcName,paramsTmp);
                event.type = WALLY_CALL_PS;
                break;
            case CALL_TYPE_CTX:
                // TODO
                params = paramsTmp;
                event.type = WALLY_CALL_CTX;
                slog(LVL_ALL,DEBUG,"call( %s(<duk_ctx *>) )",funcName,paramsTmp);
                break;
            case CALL_TYPE_PSA:
                // TODO
                // params = paramsTmp;
                slog(LVL_ALL,DEBUG,"call( %s(<PSA *>) )",funcName,paramsTmp);
                event.type = WALLY_CALL_PSA;
                break;
 
            default:
                break;
        }
        // The thread loop will free(funcName);
        event.user.data1=funcName;
        event.user.data2=params;
        // We give up the ownership of funcName + params here
        SDL_TryLockMutex(ph->funcMutex);
        SDL_PushEvent(&event);
        if(waitThread == true){
            // Enable the Mutex code for synced function calls
            slog(LVL_ALL,DEBUG,"Wait %d ms until %s has finished.",SDLWAITTIMEOUT,funcName);
            if(SDL_MUTEX_TIMEDOUT == 
                    SDL_CondWaitTimeout(ht_get_simple(ph->functionWaitConditions,funcName),ph->funcMutex,SDLWAITTIMEOUT))
                {
                    slog(LVL_ALL,ERROR,"Wait condition for call %s timed out!",funcName);
                    ph->conditionTimeout++;
                }
        }
        ret=0;
    } else {
        void *(*func)(void *) = ht_get_simple(ph->functions,funcName);
        if(func == NULL && thr_func == NULL) {
           slog(LVL_QUIET,ERROR,"Function %s not registered!",funcName);
           return false;
        }
        params = paramsTmp;
        ret = (*func)(params);
        // We give up the ownership of funcName + params here
    }
    
    return ret;
}
bool callWithData(char *funcname, void *ret, void *params){
    return callEx(funcname,ret,params,CALL_TYPE_PTR,true);
}
bool callWithString(char *funcname, void *ret, char *params){
    return callEx(funcname,ret,params,CALL_TYPE_STR,true);
}
bool call(char *funcname, void *ret, char *params){
    return callEx(funcname,ret,params,CALL_TYPE_STR,true);
}
bool callNonBlocking(char *funcname, int *ret, void *params){
    return callEx(funcname,ret,params,CALL_TYPE_PTR,false);
}

bool exportThreaded(char *name, void *f){
    if(!ht_contains(ph->functions,name,strlen(name))){
        ht_insert_simple(ph->thr_functions,name,f);
        // Enable this code for synced function calls
        ht_insert_simple(ph->functionWaitConditions, name, SDL_CreateCond());
        slog(LVL_NOISY,FULLDEBUG,"Function %s registered (threaded)",name);
        return true;
    }
    slog(LVL_QUIET,ERROR,"Function %s is already registered! Only one function allowed.",name);
    return false;
}


bool exportSync(char *name, void *f){
    if(!ht_contains(ph->functions,name,strlen(name))){
        ht_insert_simple(ph->functions,name,f);
        slog(LVL_NOISY,FULLDEBUG,"Function %s registered",name);
        return true;
    }
    slog(LVL_QUIET,ERROR,"Function %s is already registered! Only one function allowed.",name);
    return false;
}

// our own try
void wally_put_function_list(const function_list_entry *funcs) {
    const function_list_entry *ent = funcs;

    if (ent != NULL) {
        while (ent->name != NULL) {
            if(ent->threaded == true){
                slog(LVL_NOISY,DEBUG,"FKT_THRD : %s (%d args)",ent->name,ent->nargs);
                exportThreaded(ent->name,ent->value);
            } else {
                slog(LVL_NOISY,DEBUG,"FKT_SYNC : %s (%d args)",ent->name,ent->nargs);
                exportSync(ent->name,ent->value);
            }
            ent++;
        }
    }
}

bool openPlugin(char *path, char* name)
{
    char *(*initPlugin)(void *)=NULL;
    void *handle;
    char *error = NULL;
    void *nameCopy = NULL;
    asprintf((char**)&nameCopy,"%s",name);
    handle = dlopen (path, RTLD_LAZY);
    slog(LVL_INFO,INFO,"Loading plugin : %s", nameCopy);
    // Save the DL Handle for later, it has to stay open as long as we need the functions
    ht_insert_simple(ph->plugins,nameCopy,handle);
    slog(LVL_NOISY,FULLDEBUG,"Saved plugin as %s in plugin map",name);
    if (!handle) {
        slog(LVL_QUIET,ERROR,"Could not load plugin %s : %s",path,error);
        return false;
    }
    initPlugin = dlsym(handle, "initPlugin");
    if ((error = dlerror()) != NULL)  {
        slog(LVL_QUIET,ERROR,"initPlugin() failed or not found (Error : %s)",error);
        return false;
    } else {
       slog(LVL_NOISY,FULLDEBUG,"initPlugin() is now at 0x%x / handle at 0x%x",*initPlugin,handle);
    }
    // Initialize Plugin
    // TODO : Save return + function into command map
    slog(LVL_NOISY,FULLDEBUG,"Plugin loaded : %s", (*initPlugin)(ph));
    return true;
}

int cleanupPlugins(void){
    unsigned int key_count = 0;
    char *(*cleanupPlugin)(void *)=NULL;
    void **keys = ht_keys(ph->plugins, &key_count);

    for(int i=0; i < key_count; i++){
        char *name = keys[i];
        void *handle = ht_get_simple(ph->plugins,name);
        char *error = NULL;
        if (!handle) {
            slog(LVL_QUIET,ERROR,"Couldnt get plugin handle %s : %s",name,error);
            continue;
        }
        cleanupPlugin = dlsym(handle, "cleanupPlugin");
        if ((error = dlerror()) != NULL)  {
            slog(LVL_QUIET,ERROR,"cleanupPlugin(0x%x) failed or not found (Error : %s)",handle,error);
            continue;
        }
        (*cleanupPlugin)(NULL);
        // TODO : dlclose al handles in cleanup
        dlclose(handle);
    }
    return true;
}

int pluginLoader(char *path){
    DIR *d;
    struct dirent *dir;
    int pcount=0;
    d = opendir(path);
    slog(LVL_NOISY,DEBUG,"Loading plugins from folder %s",path);
    if (d) {
       while ((dir = readdir(d)) != NULL) {
          unsigned long dlen = strlen(dir->d_name);
          if(dir->d_name[dlen-3] == '.' && dir->d_name[dlen-2] == 's' && dir->d_name[dlen-1] == 'o'){
               slog(LVL_NOISY,DEBUG,"Initializing plugin %s.", dir->d_name);
               char *p = NULL;
               asprintf(&p,"%s/%s",path,dir->d_name);
               if(openPlugin(p, dir->d_name)){
                    pcount++;
               }
               free(p);
         }
       }
       closedir(d);
    } else {
        slog(LVL_QUIET,ERROR,"Could not open plugin folder %s",path);
        return -1;
    }
    slog(LVL_INFO,WARN,"Loaded %d plugins",pcount);
    ph->pluginCount = pcount;
    ph->pluginLoaderDone = true;
    return 0;
}

pluginHandler *pluginsInit(void){
    ph=malloc(sizeof(pluginHandler));
    memset(ph,sizeof(pluginHandler),0);
    if(!ph) {
        slog(LVL_QUIET,ERROR,"Could not allocate memory. Exit");
        exit(1);
    }
    // TODO : Free
    ph->mainThread = pthread_self();
    ph->pluginLoaderDone = false;

    // Setup defaults in the structure
    ph->autorender = true;
    ph->disableVideo = false;
    ph->disableAudio = false;
    ph->SDL = false;
    ph->playVideo = false;
    ph->broadcomInit = false;
    ph->daemonizing = true;
    ph->ssdp = false;
    ph->cloud = false;
    ph->location = NULL;

    ph->funcMutex = SDL_CreateMutex();
    ph->functionWaitConditions= malloc(sizeof(hash_table));
    ph->callbacks= malloc(sizeof(hash_table));
    ph->thr_functions = malloc(sizeof(hash_table));
    ph->functions = malloc(sizeof(hash_table));
    ph->plugins = malloc(sizeof(hash_table));
    ph->baseTextures = malloc(sizeof(hash_table));
    ph->fonts = malloc(sizeof(hash_table));
    ph->colors = malloc(sizeof(hash_table));
    ph->configMap = malloc(sizeof(hash_table));
    ph->configFlagsMap = malloc(sizeof(hash_table));
    ht_init(ph->functionWaitConditions, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ht_init(ph->callbacks, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ht_init(ph->thr_functions, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ht_init(ph->functions, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ht_init(ph->plugins, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ht_init(ph->baseTextures, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ht_init(ph->fonts, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ht_init(ph->colors, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ht_init(ph->configMap, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ht_init(ph->configFlagsMap, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    return ph;
}

int sendWallyCommand(char *cmd, char *log){
    FILE *f = fopen(FIFO, "a");
    if(!f) return false;
    fprintf(f, "%s %s\n",cmd, log);
    return fclose(f);
}
