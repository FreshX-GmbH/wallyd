#ifndef WALLYD_CLIENT_H
#define WALLYD_CLIENT_H

#include "../lib/slog.h"
#include "../lib/util.h"
#include "../lib/default.h"
#include "../lib/json.h"
#include "../lib/connect.h"
#include "../lib/plugins.h"
#include "dukcurl.h"

#define DEFAULT_THREAD_DELAY 2

extern bool lockSDL;
extern hash_table *commandMap;
extern hash_table *tempMap;
extern hash_table *registerMap;
extern bool registered;
extern int threadDelay;
extern char *commandURL;

bool registerClient(char *);
int sendWallyRegister();
int sendWallyLog(char *log);
bool persistConfig(hash_table *configMap);
bool sendSuccess(char *);
bool sendFailed(char *, char *);
bool getCommand(char *cmdLoc, int timeout);
bool sendCommand(char *cmdLoc, int timeout, char *cmd);

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#endif
