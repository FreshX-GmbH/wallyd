#ifndef WALLY_CONNECT_H
#define WALLY_CONNECT_H

#include <curl/curl.h>
#include "util.h"
#include "buf.h"

#define BUFFER_SIZE 32768

typedef enum {
        LONGPOLL_INIT = 0,
        LONGPOLL_GET = 1,
        LONGPOLL_POST = 2
} lpTypes;

extern char *url_call(char *url);
extern char *url_longpoll(char *url, int timeout, int type, char *cmd);

#endif
