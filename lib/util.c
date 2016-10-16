#include "util.h"
#include "plugins.h"

extern pluginHandler *ph;

int configEntries = 0;
char **ptr=NULL;
char **keys=NULL;
char **values=NULL;
char *logFile;
//struct map_t *configMap;
pthread_mutex_t logMutex=PTHREAD_MUTEX_INITIALIZER;

bool utilInit(int _loglevel, int _logmask, int _logfilemask){
   ph->loglevel = _loglevel;
   ph->logmask = _logmask;
   ph->uiAllCount = 0;
   ph->uiOwnCount = 0;
   ph->uiEventTimeout = 0;
   ph->callCount = 0;
   ph->eventDelay = 0;
   ph->textureCount = 0;
   ph->conditionTimeout = 0;
   ph->logfileHandle = stderr;
   pthread_mutex_init (&logMutex,0);
   slog_init(NULL, WALLYD_CONFDIR"/wallyd.conf", _loglevel, 0, _logmask, _logfilemask , true);
   return true;
}

//char* print_time()
//{
//    time_t t;
//    char *buf;
//
//    time(&t);
//    buf = (char*)malloc(150);
//    strftime(buf, 150, "%Y-%m-%d %H:%M:%S (%Z)", localtime(&t));
//    return buf;
//}

char *replace(const char *src, const char *from, const char *to)
{
   if(!src || !from) return NULL;
   size_t size    = strlen(src) + 1;
   size_t fromlen = strlen(from);
   size_t tolen   = strlen(to);
   char *value    = (char*)malloc(size);
   char *dst      = value;
   if ( value != NULL )
   {
      for ( ;; )
      {
         const char *match = strstr(src, from);
         if ( match != NULL )
         {
            size_t count = match - src;
            char *temp;
            size += tolen - fromlen;
            temp = (char*)realloc(value, size);
            if ( temp == NULL ) {
               free(value);
               return NULL;
            }
            dst = temp + (dst - value);
            value = temp;
            memmove(dst, src, count);
            src += count;
            dst += count;
            memmove(dst, to, tolen);
            src += fromlen;
            dst += tolen;
         }
         else /* No match found. */
         {
            strcpy(dst, src);
            break;
         }
      }
   }
   return value;
}

void test(const char *source, const char *search, const char *repl)
{
   char *after;
   after = replace(source, search, repl);
   printf("\nsearch = \"%s\", repl = \"%s\"\n", search, repl);
   if ( after != NULL )
   {
      printf("after  = \"%s\"\n", after);
      free(after);
   }
}

const char *getConfigEntry(const char *key){
   int i;
   for(i=0; i < configEntries; i++){
      if(strcmp(key,keys[i]) == 0)
            return values[i];
   }
   return NULL;
}

int getConfig(hash_table *map, const char *file)
{
//   slog(TRACE,FULLDEBUG,"Map : 0x%x, Filename : %s",map,file);
   int count=0;
   FILE *fp = fopen(file,"r");
   if (fp == NULL){
      slog(TRACE,LOG_UTIL,"Can't open file %s.", file,count);
      return 0;
   } else {
      char *line = (char*)malloc(512);
      assert(line);
      unsigned long l=0;
      while (fgets ( line, 512, fp ) != NULL ) /* read a line */
      {
//         slog(TRACE,FULLDEBUG,"Read %d bytes from %s",strlen(line),file);
         // skip comment lines
         if(line[0] == '#') continue;
         if(line[0] == ';') continue;
         if(line[0] == '/' && line[1] == '/') continue;
         l=strlen(line);
         // skip lines with less than 3 chars (no k=v assigment possible)
         if(l < 3) continue;
         // remove <RETURNs>
         line[l-1]=0; 
         if(line[l-2] == '\r' || line[l-2] == '\n')
            line[l-2]=0; 
         char *k = strtok(line, "=");
         char *v = strtok(NULL, "=");
         // Skip null values
         if(!v) continue;
         //slog(TRACE,FULLDEBUG,"K/V : %s = %s",k,v);
         unsigned long vlen=strlen(v);
         // Skip empty values
         if(vlen < 1) continue;
         if(v[0]=='"' && v[vlen-1] == '"'){
              char *vnew=malloc(vlen-1);
              memcpy(vnew,v+1,vlen-2);
              memset(v,0,vlen);
              memcpy(v,vnew,vlen-2);
              free(vnew);
         }
         if(!k || !v){
            slog(WARN,LOG_UTIL,"Line %s is not valid. Ignored.");
            continue;
         }
         ht_insert_simple(map,k,strdup(v));
         count++;
         if(count+1 > MAXCONF){
            slog(ERROR,LOG_UTIL,"MaxConf Count reached.");
            break;
         }
      }
   free(line);
   fclose ( fp );
   }
   configEntries = count;
   slog(TRACE,LOG_UTIL,"Config entries in %s : %d",file,count);
   return count;
}

void cleanupUtil(void)
{
    // TODO : find proper free method
    // if(ph->queue) priqueue_free(ph->queue);
    if(ph->functions) ht_destroy(ph->functions);
    if(ph->plugins) ht_destroy(ph->plugins);
    if(ph->configMap) ht_destroy(ph->configMap);
    if(ph->configFlagsMap) ht_destroy(ph->configFlagsMap);
    if(ph->logfile == true){
        fclose(ph->logfileHandle);
    }
    //free(ph);
    configEntries=0;
}

void cleanupWally(int s){
    void *pret;
    if(s != 0){
      slog(INFO,LOG_UTIL,"Caught signal %d", s);
    } else {
      slog(INFO,LOG_UTIL,"Cleanup wallyd");
    }
    signal(SIGINT, SIG_DFL);

    cleanupPlugins();
    fclose(ph->logfileHandle);
    unlink(FIFO);
    // after this call ph is no more available
    cleanupUtil();
    if(pthread_join(ph->uv_thr,&pret) == 0){
       free(ph->uv_thr);
    }
    exit(s);
}

int c_cleanupWally(void *s){
   cleanupWally(atoi(s));
   return 0;
}

void debugWally(int i){
   FILE *f = fopen("/tmp/wally.debug","w");
   long rss = getCurrentRSS();
   long rssPeak = getPeakRSS();
   printf("============================================================\n");
   printf("RSS Memory   : %lu (%lu kb) / Peak : %lu (%lu kb)\n",rss, rss/1024, rssPeak, rssPeak/1024);
   printf("UI All Count : %d\n",ph->uiAllCount);
   printf("UI Own Count : %d\n",ph->uiOwnCount);
   printf("UI EvTimeout : %d\n",ph->uiEventTimeout);
   printf("Cond Timeout : %d\n",ph->conditionTimeout);
   printf("UI EvDelay   : %d\n",ph->eventDelay);
   printf("CALL Count   : %d\n",ph->callCount);
   printf("Texture Count: %d\n",ph->textureCount);
   printf("Plugin Count : %d\n",ph->pluginCount);
   printf("============================================================\n");
   fprintf(f,"RSS Memory   : %lu (%lu kb) / Peak : %lu (%lu kb)\n",rss, rss/1024, rssPeak, rssPeak/1024);
   fprintf(f,"UI All Count : %d\n",ph->uiAllCount);
   fprintf(f,"UI Own Count : %d\n",ph->uiOwnCount);
   fprintf(f,"UI EvTimeout : %d\n",ph->uiEventTimeout);
   fprintf(f,"Cond Timeout : %d\n",ph->conditionTimeout);
   fprintf(f,"UI EvDelay   : %d\n",ph->eventDelay);
   fprintf(f,"CALL Count   : %d\n",ph->callCount);
   fprintf(f,"Texture Count: %d\n",ph->textureCount);
   fprintf(f,"Plugin Count : %d\n",ph->pluginCount);
   fclose(f);
   //raise(SIGUSR2);
}

void setupSignalHandler(void){
   struct sigaction sigIntHandler;
   struct sigaction sigDebugHandler;

   sigDebugHandler.sa_handler = &debugWally;
   sigemptyset(&sigDebugHandler.sa_mask);
   sigDebugHandler.sa_flags = 0;

   sigIntHandler.sa_handler = &cleanupWally;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT,  &sigIntHandler, NULL);
   sigaction(SIGUSR1, &sigDebugHandler, NULL);
}

FILE *openLogfile(char *name){
    slog(TRACE,LOG_UTIL,"Opening Logfile and redirecting logs to %s",name);
    FILE *stream = fopen(name, "a");
    setbuf(stream, NULL);
    setvbuf(stream, NULL, _IONBF, 0);
    return stream;
}

int daemonize(bool doit)
{
    if(!doit) return false;
    pid_t process_id = 0, sid = 0;
    process_id = fork();
    if (process_id < 0) {
        printf("fork failed!\n");
        exit(1);
    }
    if (process_id > 0) {
    //    printf("process_id of child process %d \n", process_id);
        exit(0);
    }
    umask(0);
    sid = setsid();
    if(sid < 0) {
        exit(1);
    }
    chdir("/");
    // Close stdin. stdout and stderr
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);
    return 0;
}

// puts the number converted of string *str into *value
// returns false if we could str is not at num or a percentage
// relativeTo is the base value to calculate the percentage of
// (i.e. 80% of 500 is 400) or the MAX value, if the given value
// is negative, its substracted from the MAX
// Note : the last char of the String must be % if percentage

int getNumOrPercentEx(char *str, int relativeTo, int *value,int base){
   int x=0;
   errno = 0;
   int err = 0;
   if(!str) {
      slog(INFO,LOG_UTIL,"getNumOrPercent() : string invalid");
      return false;
   }
   unsigned long len = strlen(str);
   if(str[len-1] == '%'){
      str[len-1] = '\0';
      if(str) x = (int)strtol(str,NULL,10);
      //else errno = 1;
      str[len-1] = '%';
      if(errno) {
         slog(WARN,LOG_UTIL,"strtol(%s) conversion error %d",str,err);
         return false;
      }
      *value = relativeTo * x / 100;
      slog(TRACE,LOG_UTIL,"it's percent : %s = %d",str,*value);
      return true;
   }
   if(str) x = (int)strtol(str,NULL,base);
   if(errno) {
         slog(WARN,LOG_UTIL,"strtol(%s) conversion error %d",str,errno);
         return false;
   }
   if(x < 0){
      *value = relativeTo + x;
   } else {
      *value = x;
   }
   return true;
}

