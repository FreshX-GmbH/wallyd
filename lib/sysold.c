#include "plugins.h"
#include "util.h"

extern pluginHandler *ph;

//  This reflects the sys plugin and its function
//  Its part of the core bin and not (un)loadable

void dumpDebug(void){
#ifdef WALLY_DMALLOC
    export("sys::debug",&dmalloc_verify);
#endif
}

void wally_sleep(char *time){
    int t = strtol(time,NULL,10);
    if(t > 0){
       slog(LVL_NOISY,FULLDEBUG,"Command processing sleeling for %d seconds",t);
       sleep(t);
       slog(LVL_NOISY,FULLDEBUG,"Command continues.");
    }
}

void wally_registerCallback(char *str){
   char *all = str;
   char *name = strtok(str," ");
   char *rest = all+strlen(name)+1;
   slog(LVL_NOISY,DEBUG,"Registering callback %s = %s",name,rest);
   ht_insert_simple(ph->callbacks,name,rest);
}

void setDebug(char *str){
    getNum(strtok(str," "),&ph->loglevel);
    slog(LVL_NOISY,DEBUG,"Setting new debug level to : %d",ph->loglevel);
}

void cleanupWally(int s){
    if(s != 0){
      slog(LVL_INFO,INFO,"Caught signal %d", s);
    } else {
      slog(LVL_INFO,INFO,"Cleanup wallyd");
    }
    signal(SIGINT, SIG_DFL);

    cleanupPlugins();
    //fclose(fifo);
    fclose(ph->logfileHandle);
    unlink(FIFO);
    // after this ph is no more available
    cleanupUtil();
#ifdef WALLY_DMALLOC
    dmalloc_shutdown();
#endif
    exit(s);
}

void initSysPlugin(void *p){
   export("quit",&cleanupWally);
   export("sys::quit",&cleanupWally);
   export("sys::setDebug",&setDebug);
   export("sys::debug",&dumpDebug);
   export("sys::sleep",&wally_sleep);
   export("sys::callback",&wally_registerCallback);
}


