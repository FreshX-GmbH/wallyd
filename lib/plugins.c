#include "util.h"
#include "dlfcn.h"
#include "plugins.h"
#include <dirent.h>
#include <SDL_events.h>
#include <SDL_mutex.h>

#define SDLWAITTIMEOUT 2000
#define SDLWAITTIMEOUT_TRANSACTION 20000

pluginHandler *ph;
pthread_mutex_t callMutex=PTHREAD_MUTEX_INITIALIZER;

// TODO : SPP compatible version (?)
bool spp_initWtx(wally_call_ctx** xwtx,int id){
    *xwtx = malloc(sizeof(wally_call_ctx));
    wally_call_ctx *wtx = *xwtx;
    memset(wtx,0,sizeof(wally_call_ctx));
    (*xwtx)->elements = 0;
    wtx->transaction = false;
    wtx->transaction_id = id;
    return true;
}

wally_call_ctx* initWtx(int id){
    wally_call_ctx *wtx = malloc(sizeof(wally_call_ctx));
    memset(wtx,0,sizeof(wally_call_ctx));
    wtx->elements = 0;
    wtx->transaction = false;
    wtx->transaction_id = id;
    if(id > 0){
      ph->transactions[id] = wtx;
    }
    return wtx;
}

bool spp_newWtx(int id, wally_call_ctx** wtx){
    if(id > MAX_WTX){
        slog(ERROR,LOG_PLUGIN,"MAX_WTX %d reached. Can not create more transactions!",MAX_WTX);
        return false;
    }
    return spp_initWtx(wtx,id);
}

wally_call_ctx* newWtx(int id){
    if(id > MAX_WTX){
        slog(ERROR,LOG_PLUGIN,"MAX_WTX %d reached. Can not create more transactions!",MAX_WTX);
        return false;
    }
    return initWtx(id);
}

// Free the WTX and ALL its elements
void freeWtxElements(wally_call_ctx* wtx){
    if(!wtx) {
       slog(ERROR,LOG_PLUGIN,"Can not free empty wtx. Fix this!");
       return;
    }
    pthread_mutex_lock(&ph->wtxMutex);
    int elements = wtx->elements, count = 0;
//    wally_call_ctx *wtx = *xwtx;
    slog(DEBUG,LOG_PLUGIN,"Free WTX %d with %d elements", wtx->transaction_id, wtx->elements);
    for(; elements >= 0; elements--){
        slog(TRACE,LOG_PLUGIN,"Free WTX element %d %s(%s)", elements,wtx->name[elements],wtx->param[elements]);
        if(wtx->name[elements]){
            count++;
            free(wtx->name[elements]);
            wtx->name[elements] = NULL;
        } else {
            slog(TRACE,LOG_PLUGIN,"Element %d already freed!",elements);
        }
        if(wtx->param[elements]){
            free(wtx->param[elements]);
            wtx->param[elements] = NULL;
        } else {
            slog(TRACE,LOG_PLUGIN,"Element parameter %d already freed!",elements);
        }
    }
    slog(DEBUG,LOG_PLUGIN,"Freed %d elements",count); wtx->elements = 0;
    pthread_mutex_unlock(&ph->wtxMutex);
}

void *freeWtx(wally_call_ctx* wtx){
    freeWtxElements(wtx);
    free(wtx);
    return NULL;
}
// make a simple context for function f
bool ssp_newSimpleWtx(wally_call_ctx** xwtx, const char *fstr,const char *params){
    *xwtx = malloc(sizeof(wally_call_ctx));
    wally_call_ctx *wtx = *xwtx;
    memset(wtx,0,sizeof(wally_call_ctx));
    wtx->name[0]=strdup(fstr);
    if(params != NULL){
        slog(TRACE,LOG_PLUGIN,"Creating simple wtx : %s %s",fstr,params);
        wtx->param[0]=strdup(params);
    }
    // also possible access
    (*xwtx)->elements = 1;
    wtx->transaction = false;
    wtx->transaction_id = 0;
    wtx->type[0] = CALL_TYPE_STR;
    return true;
}


// make a simple context for function f
wally_call_ctx* newSimpleWtx(const char *fstr,const char *params){
    wally_call_ctx *wtx = initWtx(0);
    wtx->name[0]=strdup(fstr);
    if(params != NULL){
        slog(TRACE,LOG_PLUGIN,"Creating simple wtx : %s %s",fstr,params);
        wtx->param[0]=strdup(params);
    }
    // also possible access
    wtx->elements = 1;
    wtx->transaction = false;
    wtx->transaction_id = 0;
    wtx->type[0] = CALL_TYPE_STR;
    return wtx;
}

bool pushSimpleWtx(int id, const char *fstr,const char *params){
    wally_call_ctx *wtx = ph->transactions[ph->transaction];
    slog(TRACE,LOG_PLUGIN,"WTX for cmd %s is at 0x%x",fstr,wtx);
    int idx = wtx->elements;
    wtx->name[idx]=strdup(fstr);
    if(params != NULL){
        slog(TRACE,LOG_PLUGIN,"Pushing simple wtx : %s %s",fstr,params);
        wtx->param[idx]=strdup(params);
    } else {
        wtx->param[idx]=NULL;
    }
    wtx->elements = idx + 1;
    wtx->type[idx] = CALL_TYPE_STR;
    return true;
}

bool commitWtx(int id){
    // Transaction is commited. Finish WTX and push onto priQ
    slog(DEBUG,LOG_PLUGIN,"Transaction %d commited.",id);
    wally_call_ctx *wtx = ph->transactions[id];
    wtx->transaction_id = id;
    return callEx("commit",NULL,wtx,CALL_TYPE_WTX,true);
}

bool callWtx(char *fstr, char *params){
    pthread_mutex_lock(&ph->wtxMutex);
   // No transaction running
    if(ph->transaction == false) {
        //int ret;
	    slog(DEBUG,LOG_PLUGIN,"Single WTX Call");
	    wally_call_ctx *wtx;
	    wtx = newSimpleWtx(fstr,params);
	    callEx(fstr,NULL,wtx,CALL_TYPE_WTX,true);
    } else {
       // Transaction is running, simply push the cmd to the transaction
        pushSimpleWtx(ph->transaction, fstr, params);
    }
    pthread_mutex_unlock(&ph->wtxMutex);
    return true;
}

//  Paramtyp 0 : pointer
//           1 : string
//           2 : PointerStruct *PS
//           3 : DukContext *ctx
//           4 : WallyContext *wtx

bool callEx(char *funcNameTmp, void *ret, void *paramsTmp, int paramType,bool waitThread){
    if(!funcNameTmp){
        slog(ERROR,ERROR,"Invalid function name");
        return false;
    }
    pthread_mutex_lock(&callMutex);
    void *params = NULL;
    int localret = 0;
   // wally_call_ctx *wtx = NULL;
    char *funcName = strdup(funcNameTmp);
    ph->callCount++;   
    void *(*thr_func)(void *) = ht_get_simple(ph->thr_functions,funcName);
    if(thr_func){
        SDL_Event event = { 0 }; //malloc(sizeof(SDL_Event));
        SDL_zero(event);
        switch(paramType) {
            case CALL_TYPE_PTR:
                if(paramsTmp) {
                    params = paramsTmp;
                    slog(INFO,LOG_PLUGIN,"call( %s(<binary>) )",funcName);
                    event.type = WALLY_CALL_PTR;
                } else { 
                    slog(INFO,LOG_PLUGIN,"call( %s(NULL) )",funcName);
                    event.type = WALLY_CALL_NULL;
                }
        	event.user.data2=params;
                break;
//            case CALL_TYPE_STR:
//                if(paramsTmp){
//                    params = strdup(paramsTmp);
//                    slog(DEBUG,LOG_PLUGIN,"call( %s(%s) )",funcName,params);
//                    event.type = WALLY_CALL_STR;
//                } else {
//                    event.type = WALLY_CALL_NULL;
//                }
//        	event.user.data2=params;
//                break;
//            case CALL_TYPE_PS:
//                // TODO
//                params = paramsTmp;
//                slog(DEBUG,LOG_PLUGIN,"call( %s(<PS *>) )",funcName,paramsTmp);
//        	event.user.data2=params;
//                event.type = WALLY_CALL_PS;
//                break;
//            case CALL_TYPE_CTX:
//                // TODO
//                params = paramsTmp;
//                event.type = WALLY_CALL_CTX;
//        	event.user.data2=params;
//                slog(DEBUG,LOG_PLUGIN,"call( %s(<duk_ctx *>) )",funcName,paramsTmp);
//                break;
            case CALL_TYPE_WTX:
                slog(TRACE,LOG_PLUGIN,"WTX call : %s",funcName);
                params = paramsTmp;
                event.type = WALLY_CALL_WTX;
		slog(DEBUG,LOG_PLUGIN,"Pushed 0x%x to the queue",params);
        	priqueue_insert_ptr(ph->queue,params,0, DEFAULT_PRIO);
                break;
            default:
                break;
        }
        // We give up the ownership of the funcName copy + and the params copy here
        // The ui loop will free this later, hence we do a copy for us BEFORE we push the event
        event.user.data1 = funcName;
        char *funcBak = strdup(funcName);
        SDL_TryLockMutex(ph->funcMutex);
        SDL_PushEvent(&event);
        if(waitThread == true){
            int timeout;
            if(ph->transaction == true){
                timeout = SDLWAITTIMEOUT_TRANSACTION;
            } else {
		// TODO : 
                timeout = SDLWAITTIMEOUT;
            }
            slog(DEBUG,LOG_PLUGIN,"Wait %d ms until %s has finished. Name/Cnd at 0x%x/0x%x",timeout,funcBak,funcBak,ht_get_simple(ph->functionWaitConditions,funcBak));
            if(SDL_MUTEX_TIMEDOUT == SDL_CondWaitTimeout(ht_get_simple(ph->functionWaitConditions,funcBak),ph->funcMutex,timeout)) {
                slog(ERROR,LOG_PLUGIN,"Wait condition for call %s timed out!",funcBak);
                ph->conditionTimeout++;
            }
        }
        free(funcBak);
        localret=0;
    } else {
        void *(*func)(void *) = ht_get_simple(ph->functions,funcName);
        if(func == NULL && thr_func == NULL) {
           slog(ERROR,LOG_PLUGIN,"Function %s not registered!",funcName);
           return false;
        }
        params = paramsTmp;
        localret = (*func)(params);
	free(funcName);
    }
    pthread_mutex_unlock(&callMutex); 
    //if(ret != NULL) ret = localret;
    return ret;
}
bool callWithData(char *funcname, void *ret, void *params){
    return callEx(funcname,ret,params,CALL_TYPE_PTR,true);
}
bool callSync(char *funcname, void *ret, char *params){
    return callEx(funcname,ret,params,CALL_TYPE_STR,true);
}
bool call(char *funcname, void *ret, char *params){
    slog(ERROR,LOG_PLUGIN,"call(%s(%s)) no more supported",funcname,params);
    return false;
}
bool callNonBlocking(char *funcname, int *ret, void *params){
    return callEx(funcname,ret,params,CALL_TYPE_PTR,false);
}

bool exportThreaded(const char *name, void *f){
    if(!ht_contains_simple(ph->functions,(void*)name)){
        ht_insert_simple(ph->thr_functions,(void*)name,f);
        // Enable this code for synced function calls
        ht_insert_simple(ph->functionWaitConditions, (void*)name, SDL_CreateCond());
        slog(DEBUG,LOG_PLUGIN,"Function %s registered (threaded). Condition is at 0x%x",name,ht_get_simple(ph->functionWaitConditions,(void*)name));
        return true;
    }
    slog(ERROR,ERROR,"Function %s is already registered! Only one function allowed.",name);
    return false;
}

bool exportSync(const char *name, void *f){
    //if(!ht_contains(ph->functions,name,strlen(name))){
    if(!ht_contains_simple(ph->functions,(void*)name)){
        ht_insert_simple(ph->functions,(void*)name,f);
        slog(DEBUG,LOG_PLUGIN,"Function %s registered",name);
        return true;
    }
    slog(ERROR,LOG_PLUGIN,"Function %s is already registered! Only one function allowed.",name);
    return false;
}

void wally_put_function(const char *name, int threaded, wally_c_function f, int args){
    //char *ncopy = strdup(name);
    //assert(ncopy);
    assert(ph);
    if(threaded == true){
       // TODO : free
       exportThreaded(name,f);
    } else {
       slog(DEBUG,LOG_PLUGIN,"FKT_SYNC : %s (%d args)",name,args);
       // TODO : free
       exportSync(name,f);
    }
}
// our own try
void wally_put_function_list(pluginHandler *_ph, function_list_entry *funcs) {
    if(!ph){
        slog(ERROR,LOG_PLUGIN,"PH got lost! Resetting it.");
        ph=_ph;
    }
    const function_list_entry *ent = funcs;
    if (ent != NULL) {
        while (ent->name != NULL) {
            wally_put_function(ent->name, ent->threaded, ent->value, ent->nargs);
            ent++;
        }
    }
}



bool openPlugin(char *path, char* name)
{
    char *(*initPlugin)(void *)=NULL;
    void *handle;
    char *error = NULL;
    handle = dlopen (path, RTLD_LAZY);
    ht_insert_simple(ph->plugins,name,handle);
    if (!handle) {
        slog(ERROR,LOG_PLUGIN,"Could not load plugin %s : %s",path,dlerror());
        return false;
    }
    initPlugin = dlsym(handle, "initPlugin");
    if ((error = dlerror()) != NULL)  {
        slog(ERROR,LOG_PLUGIN,"initPlugin() failed or not found (Error : %s)",error);
        return false;
    } else {
       slog(DEBUG,LOG_PLUGIN,"initPlugin() is now at 0x%x / handle at 0x%x",*initPlugin,handle);
    }
    // Initialize Plugin
    // TODO : Save return + function into command map
    char *r = (*initPlugin)(ph);
    slog(INFO,LOG_PLUGIN,"Plugin loaded : %s", r);
    return true;
}

int cleanupPlugins(void){
    unsigned int key_count = 0;
    slog(INFO,LOG_PLUGIN,"Cleaning up loaded plugins");
    char *(*cleanupPlugin)(void *)=NULL;
    int count =0;
    void **keys = malloc(sizeof(void*)*ph->plugins->count);
    count = ht_keys(ph->plugins, keys);

    for(int i=0; i < key_count; i++){
        // TODO : cleanup
        char *name = NULL;//*keys+i*sizeof(void*);
        void *handle = ht_get_simple(ph->plugins,name);
        char *error = NULL;
        if (!handle) {
            slog(ERROR,LOG_PLUGIN,"Couldnt get plugin handle %s : %s",name,error);
            continue;
        }
        cleanupPlugin = dlsym(handle, "cleanupPlugin");
        if ((error = dlerror()) != NULL)  {
            slog(ERROR,LOG_PLUGIN,"cleanupPlugin(0x%x) failed or not found (Error : %s)",handle,error);
            continue;
        }
        (*cleanupPlugin)(NULL);
        // TODO : dlclose al handles in cleanup
        dlclose(handle);
    }
    free(keys);
    free(ph->queue);
    return true;
}

int pluginLoader(void *path){
    DIR *d;
    struct dirent *dir;
    int pcount=0;
    d = opendir(path);
    slog(DEBUG,LOG_PLUGIN,"Loading plugins from folder %s",path);
    if (d) {
       while ((dir = readdir(d)) != NULL) {
          unsigned long dlen = strlen(dir->d_name);
          if(dir->d_name[dlen-3] == '.' && dir->d_name[dlen-2] == 's' && dir->d_name[dlen-1] == 'o'){
               slog(DEBUG,LOG_PLUGIN,"Initializing plugin %s.", dir->d_name);
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
        slog(ERROR,LOG_PLUGIN,"Could not open plugin folder %s",path);
        return -1;
    }
    slog(INFO,LOG_PLUGIN,"Loaded %d plugins",pcount);
    ph->pluginCount = pcount;
    ph->pluginLoaderDone = true;
    return 0;
}

int equal_string(void * a, void * b) {
    char * str1 = (char *) a;
    char * str2 = (char *) b;
    int res = strcmp(str1, str2);
    if (res == 0) {return 1;}
    else {return 0;}
}

void nop(void * a) {
    return;
}

pluginHandler *pluginsInit(void){
    
    ph=malloc(sizeof(pluginHandler));
    memset(ph,sizeof(pluginHandler),0);
    pthread_mutex_init(&callMutex,0);

    if(!ph) {
        slog(ERROR,LOG_PLUGIN,"Could not allocate memory. Exit");
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
    ph->vsync = true;
    ph->playVideo = false;
    ph->broadcomInit = false;
    ph->daemonizing = true;
    ph->ssdp = false;
    ph->cloud = false;
    ph->location = NULL;
    ph->texturePrio = NULL;
    ph->transaction = false;
    ph->transactionCount = 1;
    ph->quit = false;
    ph->transactions = malloc(sizeof(void*)*MAX_WTX);
    pthread_mutex_init(&ph->wtxMutex,0);
    pthread_mutex_init(&ph->taMutex,0);
    pthread_mutex_init(&ph->callMutex,0);

    ph->funcMutex = SDL_CreateMutex();
    //ph->functionWaitConditions= malloc(sizeof(hash_table));
    //ph->callbacks= malloc(sizeof(hash_table));
    //ph->thr_functions = malloc(sizeof(hash_table));
    //ph->functions = malloc(sizeof(hash_table));
    //ph->plugins = malloc(sizeof(hash_table));
    //ph->baseTextures = malloc(sizeof(hash_table));
    //ph->fonts = malloc(sizeof(hash_table));
    //ph->colors = malloc(sizeof(hash_table));
    //ph->configMap = malloc(sizeof(hash_table));
    //ph->configFlagsMap = malloc(sizeof(HashTable));

    ph->functionWaitConditions = hashtable_new_default(equal_string, free, nop);
    ph->callbacks              = hashtable_new_default(equal_string, free, nop);
    ph->thr_functions          = hashtable_new_default(equal_string, free, nop);
    ph->functions              = hashtable_new_default(equal_string, free, nop);
    ph->plugins                = hashtable_new_default(equal_string, free, nop);
    ph->baseTextures           = hashtable_new_default(equal_string, free, nop);
    ph->fonts                  = hashtable_new_default(equal_string, free, nop);
    ph->colors                 = hashtable_new_default(equal_string, free, nop);
    ph->configMap              = hashtable_new_default(equal_string, free, nop);
    ph->configFlagsMap         = hashtable_new_default(equal_string, free, nop);
    //ht_init(ph->functionWaitConditions, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    //ht_init(ph->callbacks, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    //ht_init(ph->thr_functions, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    //ht_init(ph->functions, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    //ht_init(ph->plugins, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    //ht_init(ph->baseTextures, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    //ht_init(ph->fonts, HT_VALUE_CONST, 0.05);
    //ht_init(ph->colors, HT_VALUE_CONST, 0.05);
    //ht_init(ph->configMap, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    //ht_init(ph->configFlagsMap, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    ph->queue = priqueue_initialize(512);
    return ph;
}

int sendWallyCommand(char *cmd, char *log){
    FILE *f = fopen(FIFO, "a");
    if(!f) return false;
    fprintf(f, "%s %s\n",cmd, log);
    return fclose(f);
}

int scall(const char *msg, ...)
{
    size_t len;
    va_list args;
    char *ap;

    /* Lock for safe */
    if (pthread_mutex_lock(&ph->callMutex)) {
        slog(ERROR,LOG_PLUGIN,"Can not lock call mutex: %d\n", errno);
        return 0;
    }
    /* Convert args */

    if(msg == NULL)
        return 0;

    va_start(args, msg);
    vasprintf(&ap, msg, args);
    va_end(args);

    len = strlen(ap);
    char *func = strsep(&ap," ");
    if(ap == NULL) {
      slog(DEBUG,LOG_PLUGIN,"Expanded string has %d bytes and splits into %s with NULL parameter. (T:%d)",len,func,ap,ph->transaction);
    } else {
      slog(DEBUG,LOG_PLUGIN,"Expanded string has %d bytes and splits into %s(%s). (T:%d)",len,func,ap,ph->transaction);
    }

    callWtx(func,ap);

    /* Done, unlock mutex */
    if (pthread_mutex_unlock(&ph->callMutex)) {
        slog(ERROR,LOG_PLUGIN,"Can not unlock call mutex: %d\n", errno);
    }
    return 0;
}


