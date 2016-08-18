#define DWALLYD
#include "wallyd.h"
#include <uv.h>

// Our global context
pluginHandler *ph;

#ifdef WITH_SEADUK
int gargc;
char **gargv;
const char *startupScript = WALLYD_CONFDIR"/wallyd.d";
pthread_t uv_thr;
#else
const char *startupScript = WALLYD_CONFDIR"/wallyd.d/main.js";
#endif

int main(int argc, char *argv[]) 
{
    duk_context *ctx;
    int ret;

    // read startup parameters
    readOptions(argc,argv);

    // init plugin system (do not load the plugins yet)
    ph = pluginsInit();
    utilInit(ph,DEFAULT_LOG_LEVEL);

    slog(LVL_INFO,INFO,"Wally Image Server R%u (Build %u) starting.",BUILD_NUMBER,BUILD_DATE);

    // assing signal handlers for ctrl+c
    setupSignalHandler();

    // read and set config
    initializeConfig();

    // daemonize if set
    daemonize(ph->daemonizing);

    // init curl lib
    url_init();

    // init SDL Lib
    if(!sdlInit()){
      exit(1);
    }

    // Tie loop and context together
    //ctx = duk_create_heap(NULL, NULL, NULL, &loop, (duk_fatal_function)my_duk_fatal);
    ctx = duk_create_heap(NULL, NULL, NULL, &loop,NULL);
    if (!ctx) {
      fprintf(stderr, "Problem initiailizing duktape heap\n");
      return -1;
    }
    ph->ctx = ctx;
    duk_push_string(ctx, "print('Hello world!');");
    duk_peval(ctx);

    if(ht_get_simple(ph->configMap,"basedir") != NULL) {
      ph->basedir=ht_get_simple(ph->configMap,"basedir");
    } else {
      asprintf(&ph->basedir,".");
    }

    // export system plugin functions
    initSysPlugin(ph);

    // load plugins
    if(ht_get_simple(ph->configMap,"plugins") != NULL) {
        callWithString("sys::loadPlugins",&ret,ht_get_simple(ph->configMap,"plugins"));
    } else {
        char *pdir;
        asprintf(&pdir,"%s/plugins",ph->basedir=ht_get_simple(ph->configMap,"basedir"));
        callWithString("sys::loadPlugins",&ret,pdir);
        free(pdir);
    }

    ht_dumpkeys(ph->configMap,"Config : ");
    ht_dumpkeys(ph->functions,"Exported sync commands : ");
    ht_dumpkeys(ph->thr_functions,"Exported async commands : ");

   // remove old socket
    ret = unlink(FIFO);
    if(ret == 0){
        slog(LVL_INFO,INFO,"Old FIFO found and removed.");
    }

#ifdef WITH_SEADUK
  slog(LVL_NOISY,DEBUG,"Seaduk initializing, ctx is at 0x%x.",ctx);
  gargc = argc;
  gargv = argv;
  if(argc < 2){
      const char *nargv[2];
      nargv[0] = argv[0];
      nargv[1] = startupScript;
      argc++;
      gargv = (char **) nargv;
  }
//  if(pthread_create(&uv_thr, NULL, &duvThread, ctx) != 0){
//    slog(LVL_ALL,ERROR,"Failed to create seaduk thread!");
//  }
#else
  if (argc < 2) {
    char *newargv[2];
    argc++;
    newargv[0]=argv[0];
    newargv[1]=startupScript;
    argv=newargv;
  }

  // Indefinetely Loop the SDL/UI thread loop in the main thread
  //
  slog(LVL_NOISY,DEBUG,"ID is : %s",argv[0]);
  slog(LVL_NOISY,DEBUG,"Startup script is : %s",argv[1]);
  // Stash argv for later access
  duk_push_pointer(ctx, (void *) argv);
  duk_push_int(ctx, argc);
  if (duk_safe_call(ctx, duv_stash_argv, 2, 1)) {
    duv_dump_error(ctx, -1);
    uv_loop_close(&loop);
    duk_destroy_heap(ctx);
    return 1;
  }
  duk_pop(ctx);

  // Initializie duv curl plugin
  dukopen_curl(ctx);

  duk_push_c_function(ctx, duv_main, 1);
  duk_push_string(ctx, argv[1]);
  // Start the JS startup script in a thread 
  slog(LVL_NOISY,FULLDEBUG,"Seaduk thread starting");
  uv_thread_create(&uv_thread, &duvThread, ctx);
#endif

  // Loop the SDL stuff in the main thread
  uiLoop(ph);

  uv_loop_close(&loop);
  duk_destroy_heap(ctx);
}

#ifndef WITH_SEADUK
void duvThread(void *ctx){
 if (duk_pcall(ctx, 1)) {
    duv_dump_error(ctx, -1);
    uv_loop_close(loop);
    duk_destroy_heap(ctx);
    return;
  }
}
#endif

void allocBuffer(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
      buf->base = malloc(size);
      buf->len = size;
      memset(buf->base,0,size);
}

void onClose(uv_handle_t* handle){
    slog(LVL_NOISY,FULLDEBUG,"Handle closed. Freeing up.");
    if(handle) { free(handle); }
}

void onNewConnection(uv_stream_t *server, int status)
{
    slog(LVL_QUIET,DEBUG,"New connection on socket "FIFO);
    if (status == -1) {
        slog(LVL_QUIET,ERROR,"New connection error on socket "FIFO);
        return;
    }
    uv_tcp_t client; //= (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(&loop, &client);
    if (uv_accept(server, (uv_stream_t*) &client) == 0) {
        uv_read_start((uv_stream_t*) &client, allocBuffer, onRead);
    }
    else {
        uv_close((uv_handle_t*) &client, NULL);
    }
//    if (uv_accept(server, (uv_stream_t*) &client) == 0) {
//        uv_os_fd_t fd;
//        uv_fileno((const uv_handle_t*) &client, &fd);
//        slog(LVL_NOISY,FULLDEBUG, "Worker %d: Accepted fd %d", getpid(), fd);
//        uv_read_start((uv_stream_t*) &client, allocBuffer, onRead);
//    } else {
//        uv_close((uv_handle_t*) &client, onClose);
//    }
}

void onRead(uv_stream_t* _client, ssize_t nread, const uv_buf_t* buffer) {
    slog(LVL_ALL,DEBUG,"onRead");
    if (nread == -1) {
        slog(LVL_NOISY,DEBUG, "Reached EOF on command socket");
        uv_close((uv_handle_t *) _client, onClose);
        return;
    }
    // TODO : Find out why this happens? (nread == -4095)
    if (strlen(buffer->base) == 0 ) {
        free(buffer->base);
        return;
    }
    slog(LVL_NOISY,DEBUG,"Read (%u/%u bytes) : %s",nread,strlen(buffer->base),buffer->base);
    if(nread > 1){
        processCommand(buffer->base);
    }
    free(buffer->base);
    slog(LVL_NOISY,DEBUG,"Closing handle");
    uv_close((uv_handle_t*) _client, onClose);
}

bool processCommand(char *buf)
{
    int ret;
    int validCmd = 0;
    bool nextLine = true;
    char *lineBreak, *spaceBreak;
    char *lineCopy = NULL;
    char *cmd = strtok_r(buf,"\n",&lineBreak);
    while( nextLine ){
        // TODO : Keep track of this and clean it up!
        unsigned long cmdLen = strlen(cmd);
        lineCopy = malloc(cmdLen+1);
        memset(lineCopy, 0, cmdLen+1);
        strncpy(lineCopy,cmd,cmdLen);
        slog(LVL_NOISY,FULLDEBUG,"Processing line (%d) : %s",cmdLen,lineCopy);
        // NOTE : strtok changes strlen of cmd, so we save its length before
        if(cmd[0] != '#') {
            char *myCmd = strtok_r(lineCopy, " ", &spaceBreak);
            char *params=NULL;
            if(cmdLen > strlen(myCmd)){
                params = lineCopy+strlen(myCmd)+1;
            }
            slog(LVL_ALL,DEBUG,"Command split into(%d) : %s(%s)",strlen(myCmd), myCmd, params);
            if(callWithString(myCmd,&ret, params)){
                validCmd++;
            }
        } else {
            slog(LVL_NOISY,FULLDEBUG,"Ignoring comment line");
        }
        cmd = strtok_r(NULL,"\n",&lineBreak);
        if(cmd == NULL) nextLine=false;
        free(lineCopy);
    }
    slog(LVL_NOISY,FULLDEBUG,"Command stack executed.");
    return validCmd;
}

void initializeFlags(void){
   slog(LVL_NOISY,FULLDEBUG,"Initializing flags file");
   // if /etc/wally.conf has h/w defined dont set DEFAULT_W/H
   if(getConfig(ph->configFlagsMap,ETC_FLAGS) == 0){
      slog(LVL_NOISY,DEBUG,"Trying to open "ETC_FLAGS);
      if(getConfig(ph->configFlagsMap,ETC_FLAGS_BAK) == 0){
        slog(LVL_INFO,WARN,"Configfile in "ETC_FLAGS" nor "ETC_FLAGS_BAK" not found! Using default values.");
        ph->width = ph->width ? ph->width : DEFAULT_WINDOW_WIDTH;
        ph->height = ph->height ? ph->height : DEFAULT_WINDOW_HEIGHT;
      }
  }

  // params from /tmp/flags can still override w/h from wally.conf
  if(ht_contains_simple(ph->configFlagsMap,"W_WIDTH")){
      ph->width = atoi(ht_get_simple(ph->configFlagsMap,"W_WIDTH"));
      ph->width = ph->width ? ph->width : DEFAULT_WINDOW_WIDTH;
  }
  if(ht_get_simple(ph->configFlagsMap,"W_HEIGHT")){
      ph->height = atoi(ht_get_simple(ph->configFlagsMap,"W_HEIGHT"));
      ph->height = ph->height ? ph->height : DEFAULT_WINDOW_HEIGHT;
  }
  slog(LVL_NOISY,DEBUG,"W = %d / H = %d",ph->width,ph->height);
  if(!ht_contains_simple(ph->configFlagsMap,"W_MAC")){
     slog(LVL_QUIET,ERROR,"No MAC address found in configs/flags. Can not determine uuid for this device");
     ht_insert_simple(ph->configFlagsMap,"W_MAC",DEFAULT_MAC);
     slog(LVL_NOISY,DEBUG,"Setting MAC to "DEFAULT_MAC);
     ph->uuid = DEFAULT_MAC;
  }
  ph->uuid = replace(ht_get_simple(ph->configFlagsMap,"W_MAC"),":","");
  slog(LVL_NOISY,DEBUG,"UUID is : %s",ph->uuid);

  if(ht_contains_simple(ph->configFlagsMap,"W_CONNECT")){
      slog(LVL_NOISY,DEBUG,"Connectivity type is : %s",ht_get_simple(ph->configFlagsMap,"W_CONNECT"));
      if(strncmp(ht_get_simple(ph->configFlagsMap,"W_CONNECT"),"ssdp",4)){
        ph->ssdp = true; 
      }
      if(strncmp(ht_get_simple(ph->configFlagsMap,"W_CONNECT"),"cloud",5)){
        ph->cloud = true; 
      }
      if(strncmp(ht_get_simple(ph->configFlagsMap,"W_CONNECT"),"manual",6)){
        if(!ht_contains_simple(ph->configFlagsMap,"W_SERVER")){
          slog(LVL_QUIET,ERROR,"Connectivity type set to manual but no server defined");
        }
      }
  }
}

void initializeConfig(void){
  initializeFlags();

  //ph->configMap=map_create();
  if(getConfig(ph->configMap,WALLYD_CONFIG) > 0){
    if(ht_contains_simple(ph->configMap,"threadDelay")) { 
      ph->threadDelay = atoi(ht_get_simple(ph->configFlagsMap,"threadDelay"));
    }
    if(ht_contains_simple(ph->configMap,"raspberry") && strncmp(ht_get_simple(ph->configMap,"raspberry"),"true",4) == 0) { 
	ph->broadcomInit = true; 
        slog(LVL_NOISY,DEBUG,"BCM Chip support enabled.");
    } else {
	ph->broadcomInit = false; 
    }
    if(ht_contains_simple(ph->configMap,"disableAudio") && strncmp(ht_get_simple(ph->configMap,"disableAudio"),"true",4) == 0) { 
	ph->disableAudio = true; 
        slog(LVL_NOISY,DEBUG,"Audio stream part of video playing disabled.");
    }
    if(ht_contains_simple(ph->configMap,"disableVideo") && strncmp(ht_get_simple(ph->configMap,"disableVideo"),"true",4) == 0) { 
	ph->disableVideo = true; 
        slog(LVL_NOISY,DEBUG,"Video stream part of video playing disabled.");
    }
    if(ht_contains_simple(ph->configMap,"disableVideoPQ") && strncmp(ht_get_simple(ph->configMap,"disableVideoPQ"),"true",4) == 0) { 
	ph->disableVideoPQ = true; 
        slog(LVL_NOISY,DEBUG,"Video stream Queue of video playing disabled.");
    }
    if(ht_contains_simple(ph->configMap,"disableVideoDisplay") && strncmp(ht_get_simple(ph->configMap,"disableVideoDisplay"),"true",4) == 0) { 
	ph->disableVideoDisplay = true; 
        slog(LVL_NOISY,DEBUG,"Video stream display of video playing disabled.");
    }
    if(ht_get_simple(ph->configMap,"foreground") != NULL && strncmp(ht_get_simple(ph->configMap,"foreground"),"true",4) == 0) { 
	ph->daemonizing = false; 
    }
    if(ht_get_simple(ph->configMap,"ssdp") != NULL && strncmp(ht_get_simple(ph->configMap,"ssdp"),"true",4) == 0) { 
	ph->ssdp = true; 
    }
    if(ht_get_simple(ph->configMap,"logfile") != NULL) { 
        ph->logfileHandle = openLogfile(ht_get_simple(ph->configMap,"logfile"));
    }
    if(ht_get_simple(ph->configMap,"debug") != NULL) { 
        ph->loglevel = atoi(ht_get_simple(ph->configMap,"debug"));
	slog(LVL_NOISY,DEBUG,"Set loglevel to : %d",ph->loglevel);
    }
    if(ht_get_simple(ph->configMap,"width") != NULL) { 
        ph->width = atoi(ht_get_simple(ph->configMap,"width"));
	slog(LVL_NOISY,DEBUG,"Set Window width to : %d",ph->width);
    }
    if(ht_get_simple(ph->configMap,"height") != NULL) { 
        ph->height = atoi(ht_get_simple(ph->configMap,"height"));
	slog(LVL_NOISY,DEBUG,"Set Window height to : %d",ph->height);
    }
  } else {
    slog(LVL_QUIET,ERROR,"Configfile "ETC_FLAGS" not found. Using defaults.");
  }
}

void readOptions(int argc, char **argv){
    char *cvalue = NULL;
    int c;
    while ((c = getopt (argc, argv, "fhc:s:")) != -1)
        switch (c){
            case 'h':
                printf("Usage: wallyd [-h|-f|-s <startscript>|-c <configfile>]\n");
                printf("\t-h : this help\n");
                printf("\t-f : run in foreground\n");
                printf("\t-c : use <configfile> (default "WALLYD_CONFIG")\n");
                printf("\t-s : run <startscript> (default "WALLYD_CONFDIR"/wallyd.startup)\n");
                exit(0);
                break;
            case 'f':
                slog(LVL_INFO,INFO,"Running in foreground");
                ph->daemonizing = false;
                break;
            case 's':
                startupScript = optarg;
                slog(LVL_INFO,INFO,"Using startscript %s",startupScript);
                break;
            case 'c':
                cvalue = optarg;
                slog(LVL_INFO,INFO,"Using config %s",cvalue);
                break;
            case '?':
              if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
              else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
              else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
              return;
            default:
              abort();
        }
}

void processStartupScript(char *file){
  slog(LVL_NOISY,DEBUG,"Reading wallyd.startup script : %s",file);
  long fsize=0;
  char *cmds=NULL;

  FILE *f = fopen(file, "rb");
  if(!f){
      slog(LVL_NOISY,DEBUG,"File not found. Not running any startup commands");
      return;
  }

  fseek(f, 0, SEEK_END);
  fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  cmds = malloc(fsize + 1);
  fread(cmds, fsize, 1, f);
  fclose(f);

  cmds[fsize] = 0;
  slog(LVL_NOISY,DEBUG,"Processing %d bytes from startupScript",fsize);
  processCommand(cmds);
  free(cmds);
}

static void my_duk_fatal(duk_context *ctx, int code, const char *msg){
    slog(LVL_QUIET,ERROR,"JS encountered a fatal error %d : %s",code,msg);
    slog(LVL_QUIET,ERROR,"We will continue but the JS core might be unstable");
}

