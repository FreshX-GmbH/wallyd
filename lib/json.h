#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "jsmn.h"
#include "hashtable.h"

char * json_fetch(char *url);
jsmntok_t * json_tokenise(char *js);
bool json_token_streq(char *js, jsmntok_t *t, char *s);
char * json_token_tostr(char *js, jsmntok_t *t);
bool parseJSON(hash_table*, char *, char *);
