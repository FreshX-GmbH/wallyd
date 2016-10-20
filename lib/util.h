#ifndef WALLY_UTIL_H
#define WALLY_UTIL_H

#define _GNU_SOURCE

#include "autoversion.h"
//#include "json.h"
//#include "miniz.h"
#include "Hash_Table.h"
#include "plugins.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h> 
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <duktape.h>
//#include <refs.h>

#ifndef _WIN32
#include <sys/time.h>
#else
#include <time.h>
#include <windows.h>
#include <winsock.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "slog.h"

#define DEFAULT_LOCATION_FILE "/tmp/wally.loc"
#define STAMPFONT_FILE "/root/fonts/umbrage2.ttf"
#define LOGFONT_FILE "/root/fonts/Lato-Bol.ttf"

// Maximal items to read from config file and store
#define MAXCONF 1024
// Maximal json items to parse and store
#define JSONMAXCONF 2048

#define FATAL       LVL_FATAL
#define ERROR       LVL_ERROR
#define WARN        LVL_WARN
#define INFO        3
#define DEBUG       LVL_DEBUG
#define TRACE       LVL_TRACE
#define FULLDEBUG   LVL_TRACE
#define LVL_QUIET   LVL_ERROR

#define LVL_FATAL     16384
#define LVL_ERROR     16384+1
#define LVL_WARN      16384+2
#define LVL_DEBUG     16384+4
#define LVL_TRACE     16384+5

// Change to 0 or 1 in production
//#ifndef __FULLDEBUG
//#define DEFAULT_LOG_LEVEL 3
//#else
#define DEFAULT_LOG_LEVEL 5
//#endif
//#define log(...) log_print(__LINE__, __FILE__, __VA_ARGS__ )
//#define log(...) log4c_log(__LINE__,__FILE__,__VA_ARGS__)
//#define log(level,...) slog(0,level,__VA_ARGS__)

// UTIL
char *replace(const char *src, const char *from, const char *to);
const char *getConfigEntry(const char *key);
int getConfig(HashTable *, const char *);
void cleanupUtil(void);
void cleanupWally(int);
void setupSignalHandler(void);
void log_print(int line, const char *filename, int level, char *fmt,...);
bool utilInit(int,int,int);
int daemonize(bool);

#define getNumOrPercent(a,b,c) getNumOrPercentEx(a,b,c,10)
#define getNumHex(a,c) getNumOrPercentEx(a,0,(int *)c,16)
#define getNum(a,c) getNumOrPercentEx(a,0,(int *)c,10)
int getNumOrPercentEx(char *str, int relativeTo, int *value,int base);

void *getVFSFile(pluginHandler *ph,char *file, size_t *pSize);
int openVFS(pluginHandler *ph, char *file);
//void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name, size_t *pSize, mz_uint zip_flags);
size_t getPeakRSS();
size_t getCurrentRSS();
void setupSignalHandler(void);
int c_cleanupWally(void *s);

#endif
