#ifndef WALLYD_SSDP_H
#define WALLYD_SSDP_H

#include "../lib/util.h"
#include "../lib/default.h"

#define SSDP_ST_MATCH "freshx:wally"

//extern bool ssdpThreadRunning;

int sendWallyLog(char *log);
int sendBroadcastPacket(void);

#endif
