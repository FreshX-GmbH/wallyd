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

bool utilInit(void *_ph, int _loglevel){
   ph->loglevel = _loglevel;
   ph->uiAllCount = 0;
   ph->uiOwnCount = 0;
   ph->uiEventTimeout = 0;
   ph->callCount = 0;
   ph->eventDelay = 0;
   ph->textureCount = 0;
   ph->conditionTimeout = 0;
   ph->logfileHandle = stderr;
   pthread_mutex_init (&logMutex,0);
   slog_init(NULL, NULL, _loglevel, 0, 1);
   return true;
}

char* print_time()
{
    time_t t;
    char *buf;

    time(&t);
    buf = (char*)malloc(150);
    strftime(buf, 150, "%Y-%m-%d %H:%M:%S (%Z)", localtime(&t));
    return buf;
}

//void log4c_log(int line, char *file,int level, char *fmt,...){
//   va_list argp;
////   printf("[%s:%d]",file,line);
//   va_start(argp, fmt);
//   log4c_category_vlog(ph->logCat, level, fmt, argp);
//   va_end(argp);
//}

void log_print(int line, const char *filename, int level, char *fmt,...)
{
    if(ph->loglevel < level){
      return; 
    }

    va_list list;
    char *p, *r;
    int e;
    char *lvlString = "???";

    pthread_mutex_lock(&logMutex);

    char *timeBuf = print_time();

    if(level == HARDCORE)
        lvlString = "hardcore";
    if(level == FULLDEBUG)
        lvlString = "fulldebug";
    if(level == DEBUG)
        lvlString = "debug";
    if(level == INFO)
        lvlString = "info";
    if(level == ERROR)
        lvlString = "error";
    if(level == WARN)
        lvlString = "warn";
    if(ph->loglevel < DEBUG){ 
      fprintf(ph->logfileHandle,"[%s] ",timeBuf);
    } else {
        if(ph->loglevel < FULLDEBUG){ 
            fprintf(ph->logfileHandle,"[%s][%s in %s:%d] ",lvlString, timeBuf, filename, line);
        } else {
            fprintf(ph->logfileHandle,"[0x%x][%s][%s in %s:%d] ",(unsigned int)pthread_self(),lvlString, timeBuf, filename, line);
        }
    }
    va_start( list, fmt );
 
    for ( p = fmt ; *p ; ++p )
    {
        if ( *p != '%' )//If simple string
        {
            fputc( *p,ph->logfileHandle );
        }
        else
        {
            switch ( *++p )
            {
                /* string */
            case 's':
            {
                r = va_arg( list, char * );

                if(r == NULL) {
                  fprintf(ph->logfileHandle,"%s", "(null)");
                } else {
                  fprintf(ph->logfileHandle,"%s", r);
                }
                continue;
            }
 
            case 'u':
            {
                e = va_arg( list, int );
                fprintf(ph->logfileHandle,"%u", e);
                continue;
            }
            // char
            case 'c': {
                e = va_arg( list, int );
                fprintf(ph->logfileHandle,"%c", e);
                continue;
            }
            /* float / double */
            case 'f': {
                double d = va_arg( list, double );
                fprintf(ph->logfileHandle,"%f", d);
                continue;
            }
            /* integer */
            case 'd': {
                e = va_arg( list, int );
                fprintf(ph->logfileHandle,"%d", e);
                continue;
            }
            case 'x': {
                void *p = va_arg( list, void * );
                fprintf(ph->logfileHandle,"%x", p);
                continue;
            }
             case 'p': {
                void *p = va_arg( list, void *);
                fprintf(ph->logfileHandle,"%p", p);
                continue;
            }
 
            default:
                fputc( *p, ph->logfileHandle );
            }
        }
    }
    va_end( list );
    fputc( '\n', ph->logfileHandle );
    free(timeBuf);
    pthread_mutex_unlock(&logMutex);
}

char *replace(const char *src, const char *from, const char *to)
{
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
   slog(LVL_NOISY,FULLDEBUG,"Map : 0x%x, Filename : %s",map,file);
   int count=0;
   FILE *fp = fopen(file,"r");
   if (fp == NULL){
      slog(LVL_NOISY,DEBUG,"Can't open file %s.", file,count);
      return 0;
   } else {
      char *line = (char*)malloc(512);
      assert(line);
      unsigned long l=0;
      while (fgets ( line, 512, fp ) != NULL ) /* read a line */
      {
         slog(LVL_NOISY,FULLDEBUG,"Read %d bytes from %s",strlen(line),file);
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
         //slog(LVL_NOISY,FULLDEBUG,"K/V : %s = %s",k,v);
         unsigned long vlen=strlen(v);
         // Skip empty values
         if(vlen < 1) continue;
         if(v[0]=='"' && v[vlen-1] == '"'){
              char *vnew=malloc(vlen-1);
              memcpy(vnew,v+1,vlen-2);
              memset(v,0,vlen);
              memcpy(v,vnew,vlen-2);
              free(vnew);
              slog(LVL_NOISY,FULLDEBUG,"Removed leading and trailing \" from value. its now : %s",v);
         }
         slog(LVL_NOISY,FULLDEBUG,"Splitting cleaned line %s into key %s = %s",line,k,v);
         if(!k || !v){
            slog(LVL_INFO,WARN,"Line %s is not valid. Ignored.");
            continue;
         }
         ht_insert_simple(map,k,v);
         //map_set(map, k,v);
         line=(char*)malloc(512);
         count++;
         if(count+1 > MAXCONF){
            slog(LVL_QUIET,ERROR,"MaxConf Count reached.");
            break;
         }
      }
   fclose ( fp );
   }
   configEntries = count;
   slog(LVL_NOISY,DEBUG,"Config entries in %s : %d",file,count);
   return count;
}

void cleanupUtil(void)
{
    if(ph->functions) ht_destroy(ph->functions);
    if(ph->plugins) ht_destroy(ph->plugins);
    //if(ph->configMap) map_free(ph->configMap);
    //if(ph->configFlagsMap) map_free(ph->configFlagsMap);
    if(ph->logfile == true){
        fclose(ph->logfileHandle);
    }
    //free(ph);
    configEntries=0;
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
    exit(s);
}

void debugWally(void){
   FILE *f = fopen("/tmp/wally.debug","w");
   size_t rss = getCurrentRSS();
   printf("============================================================\n");
   printf("RSS Memory   : %d (%d kb) / Peak : %d (%d kb)\n",rss, rss/1024, getPeakRSS(), getPeakRSS()/1024);
   printf("UI All Count : %d\n",ph->uiAllCount);
   printf("UI Own Count : %d\n",ph->uiOwnCount);
   printf("UI EvTimeout : %d\n",ph->uiEventTimeout);
   printf("Cond Timeout : %d\n",ph->conditionTimeout);
   printf("UI EvDelay   : %d\n",ph->eventDelay);
   printf("CALL Count   : %d\n",ph->callCount);
   printf("Texture Count: %d\n",ph->textureCount);
   printf("Plugin Count : %d\n",ph->pluginCount);
   printf("============================================================\n");
   fprintf(f,"RSS Memory   : %d (%d kb) / Peak : %d (%d kb)\n",rss, rss/1024, getPeakRSS(), getPeakRSS()/1024);
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
    slog(LVL_NOISY,DEBUG,"Opening Logfile and redirecting logs to %s",name);
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
      slog(LVL_INFO,WARN,"getNumOrPercent() : string invalid");
      return false;
   }
   unsigned long len = strlen(str);
   if(str[len-1] == '%'){
      str[len-1] = '\0';
      if(str) x = (int)strtol(str,NULL,10);
      //else errno = 1;
      str[len-1] = '%';
      if(errno) {
         slog(LVL_INFO,WARN,"strtol(%s) conversion error %d",str,err);
         return false;
      }
      *value = relativeTo * x / 100;
      slog(LVL_NOISY,FULLDEBUG,"it's percent : %s = %d",str,*value);
      return true;
   }
   if(str) x = (int)strtol(str,NULL,base);
   if(errno) {
         slog(LVL_INFO,WARN,"strtol(%s) conversion error %d",str,errno);
         return false;
   }
   if(x < 0){
      *value = relativeTo + x;
   } else {
      *value = x;
   }
   return true;
}

