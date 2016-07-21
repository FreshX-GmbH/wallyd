#include "client.h"
#define PLUGIN_SCOPE "client"

bool clientThreadRunning = false;
bool registered = false;
bool run_callback = false;
const char *cb;
int slp=1;
pthread_mutex_t logMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t commandMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_t thr;

duk_context *ctx;

int finishCallback;
const duk_function_list_entry clientMethods[];

extern pluginHandler *ph;
int saveLocation(char *);
int threadDelay = DEFAULT_THREAD_DELAY;
typedef enum { DISCOVERY, REGISTER, COMMAND, QUIT } thrState;
thrState clientState = DISCOVERY; 

//  Loop the client thread here
void *wallyClientThread(void *argPtr){
   slog(LVL_NOISY,DEBUG,"WallyClient Thread started. Waiting for plugins to get ready.");
   pthread_mutex_init(&commandMutex,0);
   while(ph->pluginLoaderDone == false){
      usleep(100);
   }
   slog(LVL_NOISY,FULLDEBUG,"WallyClient Thread started. Plugins ready.");
   commandMap = malloc(sizeof(hash_table));
   ht_init(commandMap, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
   tempMap = malloc(sizeof(hash_table));
   ht_init(tempMap, HT_KEY_CONST | HT_VALUE_CONST, 0.05);

   int startDelay = 1;
   int ret=0;
   clientThreadRunning = true;

   while(clientThreadRunning == true) {
       switch(clientState)
           {
              case DISCOVERY:
                  //ssdpDiscovery(1);
                  //    Both plugins (ssdp and cloudConnector) 
                  //    set ph->location and return once they registered
                  //    Cloud connector needs additional handling after
                  if(ph->ssdp) {
                      call("ssdp::discovery",&ret,&startDelay);
                  } else if(ph->cloud) {
                      call("cloud::connect",&ret,&startDelay);
                  }     
                  if(ph->location) {
                     if(run_callback){
                        slog(LVL_NOISY,DEBUG,"Executing JS Callback for client discovery");
                        duv_push_ref(ctx, finishCallback);
                        duk_push_string(ctx, ph->location);
                        duk_call(ctx, 1);
                        duk_to_int(ctx, -1);
                     }
                     saveLocation(FLAGFILE);
                  }
                  startDelay = 1;
                  slog(LVL_NOISY,DEBUG,"Change state from discovery to register");
                  clientState = REGISTER;
                  break;
              case REGISTER:
                  if(ph->location){
                     if(registerClient(ph->location)){
                        slog(LVL_NOISY,DEBUG,"Starting longpoll."); 
                        url_longpoll(commandURL,60,LONGPOLL_INIT,NULL);
                        clientState = COMMAND;
                     } else {
                        slog(LVL_NOISY,DEBUG,"Registration failed."); 
                     }
                  } else {
                     slog(LVL_INFO,WARN,"Register was called without a valid location");
                  }
                  break;
              case COMMAND:
                  slog(LVL_NOISY,DEBUG,"Requesting next core command from %s",commandURL);
                  int ret = getCommand(commandURL,60);
                  if(ret == true && ht_get_simple(commandMap,"command")){
                     int commandValid = false;
                     slog(LVL_NOISY,DEBUG,"Command : %s",ht_get_simple(commandMap,"command"));
                     // Handle the commands which are supported by the wallyd here
                     if(ht_compare(commandMap,"command","config")){
                           commandValid = true;
                           slog(LVL_NOISY,DEBUG,"Preparing to persist config");
                           if(persistConfig(registerMap)){
                              sendSuccess(ht_get_simple(commandMap,"id"));
                           }
                     }
                     if(ht_compare(commandMap,"command","reboot")){
                           commandValid = true;
                           sendSuccess(ht_get_simple(commandMap,"id"));
                           slog(LVL_NOISY,DEBUG,"Preparing to reboot");
                           system(BIN_REBOOT);
                     }
                     if(ht_compare(commandMap,"command","firmwareUpdate")){
                           commandValid = true;
                           sendSuccess(ht_get_simple(commandMap,"id"));
                           slog(LVL_NOISY,DEBUG,"Preparing to update firmware");
                           system(BIN_UPDATEFW);
                     }
                     if(ht_compare(commandMap,"command","getlog")){
                           commandValid = true;
                           slog(LVL_NOISY,DEBUG,"Preparing to send log to server");
                           //sendSuccess(ht_get_simple(commandMap,"id"));
                           //persistConfig(registerMap);
                     }
                     if(commandValid == false){
                           slog(LVL_NOISY,DEBUG,"Command %s not valid. Ingoring.",ht_get_simple(commandMap,"command"));
                           sendFailed(ht_get_simple(commandMap,"id"),"unknown.command");
                     }
                  } else {
                     slog(LVL_INFO,WARN,"getCommand failed.");
                  }
                  pthread_mutex_unlock(&commandMutex);
                  break;
              case QUIT:
                  slog(LVL_NOISY,DEBUG,"Thread is quiting.");
                  break;
           }
       sleep(threadDelay);
   }
   return 0;
}

int saveLocation(char *filename){
    FILE *f = fopen(filename, "a");
    if(!f){
       slog(LVL_QUIET,ERROR,"Could not save registration server to %s", filename);
       return false;
    }
    slog(LVL_QUIET,ERROR,"Saved registration server");
    fprintf(f, "W_SERVER=%s/register\n",ph->location);
    return fclose(f);
}

int sendWallyRegister(){
    FILE *f = fopen(FIFO, "a");
    if(!f) return false;
    fprintf(f, "register\n");
    return fclose(f);
}

void setDiscovery(void)
{
    if(ph->ssdp && ph->clientThreadRunning != true){
       slog(LVL_INFO,INFO,"Wallaby Server discovery running.");
       int slp=1;
       ph->ssdpRunning = false;
       run_callback = false;
       clientState = DISCOVERY;
       if(pthread_create(&thr, NULL, wallyClientThread, &slp) != 0){
          slog(LVL_QUIET,ERROR,"Failed to start the SSDP Discovery thread!");
       } else {
          ph->clientThreadRunning = true;
       }
    }
}
void setRegister(void *loc)
{
   if(ph->location || loc){
      // An valid url needs at least 7 chars
      if(loc && strlen(loc)>1){
         if(strncmp(loc,"http",4) == 0){
            ph->location = loc;
         } else {
            slog(LVL_INFO,INFO,"Registration URL %s is not valid (must start with http(s)",loc);
            return;
         }
      }
      ph->ssdpRunning = false;
      slog(LVL_INFO,INFO,"Initializing client registration at %s",loc);
      clientState = REGISTER;
   } else {
      slog(LVL_INFO,INFO,"Can not initialize client registration, invalid configserver location.");
   }
}

bool getCommand(char *cmdLoc, int timeout)
{
    return parseJSON(commandMap,url_longpoll(cmdLoc,timeout,LONGPOLL_GET,NULL),NULL);
}
bool sendCommand(char *cmdLoc, int timeout, char *cmd)
{
    return parseJSON(commandMap,url_longpoll(cmdLoc,timeout,LONGPOLL_POST,cmd),NULL);
}

bool sendFailed(char *cmdid,char *err)
{
     char *failURL=NULL;
     slog(LVL_INFO,INFO,"Command failed. Notifying server.");
     asprintf(&failURL,"%s/commandfailed?uuid=%s&cmdid=%s&err=%s",ph->location,ph->uuid,cmdid,err);
     slog(LVL_NOISY,DEBUG,"URL : %s",failURL);
     char *retString = url_call(failURL);
     free(failURL);
     return true;
}

bool sendSuccess(char *cmdid)
{
     slog(LVL_INFO,INFO,"Command executed successfully. Notifying server.");
     char *successURL=NULL;
     asprintf(&successURL,"%s/commandsuccess?uuid=%s&cmdid=%s",ph->location,ph->uuid,cmdid);
     slog(LVL_NOISY,DEBUG,"URL : %s",successURL);
     char *retString = url_call(successURL);
     free(successURL);
     return true;
}

char *cleanupPlugin(void *p){
   slog(LVL_INFO,INFO,"Cleanup client plugin. Waiting for threads to finish");
   clientThreadRunning = false;
   ph->ssdpRunning = false;
   pthread_join(thr, NULL);
   return NULL;
}

duk_ret_t js_cloudConnect(duk_context *ctx)
{
   int ret;
   int startDelay = 1;
   call("cloud::connect",&ret,&startDelay);
   return 0;
}

duk_ret_t js_discovery(duk_context *ctx)
{
   int ret;
   int startDelay = 1;
   ph->ssdp = true;
   const char *str = duk_require_string(ctx, 0);
   if (!duk_is_function(ctx, 1)) {                                            
       duk_error(ctx, DUK_ERR_TYPE_ERROR, "Function required for callback");
   }        
   duk_dup(ctx, 1);
   finishCallback = duv_ref(ctx); 

   setDiscovery();
   run_callback = true;

   slog(LVL_NOISY,DEBUG,"SSDP callback saved.");
   return 0;
}

duk_ret_t js_register(duk_context *ctx)
{
   slog(LVL_NOISY,DEBUG,"Register comes here.");
   return 0;
}

duk_ret_t js_client_dtor(duk_context *ctx)
{
   slog(LVL_NOISY,DEBUG, "Cloud object destroyed.");
   return 0;
}

duk_ret_t js_client_ctor(duk_context *ctx)
{
    slog(LVL_NOISY,DEBUG, "New cloud object created.");
    duk_push_this(ctx);
    duk_dup(ctx, 0);  /* -> stack: [ name this name ] */
    duk_put_prop_string(ctx, -2, "name");  /* -> stack: [ name this ] */

    return 0;
}

void js_client_init(duk_context *ctx) {
   slog(LVL_NOISY,DEBUG,"Constructing cloud object");
   
   duv_ref_setup(ctx);

   duk_push_c_function(ctx, js_client_ctor, 5 );

   /* Push Client.prototype object. */
   duk_push_object(ctx);

   duk_put_function_list(ctx, -1, clientMethods);
   duk_put_prop_string(ctx, -2, "prototype");
   /* Finally, register Client to the global object */
   duk_put_global_string(ctx, "CloudClient");  /* -> stack: [ ] */
}

// Client object methods
const duk_function_list_entry clientMethods[] = {
    { "discovery",         js_discovery,  2   },
    { "cloudConnect",      js_cloudConnect, 0 },
    { "register",          js_register,   0   },
    { NULL,           NULL,        0 }
};

const function_list_entry c_clientMethods[] = {
    { PLUGIN_SCOPE"::discovery", WFUNC_SYNC,  setDiscovery,  2   },
    { PLUGIN_SCOPE"::register",  WFUNC_SYNC,  setRegister,   0   },
    { NULL,    0,       NULL,        0 }
};


char *initPlugin(pluginHandler *phptr){
    ph=phptr;
    ctx = ph->ctx;
    slog(LVL_NOISY,FULLDEBUG,"Plugin client initialized. PH is at 0x%x",ph);
    wally_put_function_list(c_clientMethods);
    js_client_init(ph->ctx);
    return PLUGIN_SCOPE;
}
