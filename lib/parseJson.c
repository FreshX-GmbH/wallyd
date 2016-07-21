#include <curl/curl.h>
#include "util.h"

char *stateString[] = { "START", "KEY", "PRINT", "SKIP", "STOP" };

int jsonEntries = 0;

bool parseJSON(hash_table *map, char *js, char *inheritKey)
{
    char *str = NULL;
    char *currentKey = NULL;
    char *storeKey = NULL;
    char *parentKey = NULL;

    int storeCount=0;
    int arrayCount=0;

    if(js == NULL) {
       return NULL;
    } else {
         slog(LVL_NOISY,DEBUG,"JSON RAW : %s",js);
    }
    if(inheritKey){
        slog(LVL_NOISY,DEBUG,"inheritKey set %s", inheritKey);
    }

    jsmntok_t *tokens = json_tokenise(js);

    typedef enum { START, KEY, PRINT, SKIP, STOP } parse_state;
    parse_state state = START;

    size_t object_tokens = 0;
    int childSize=0;
    int inheritLoop=0;
    int parentKeyCount = 0;

    for (size_t i = 0, j = 1; j > 0; i++, j--)
    {
        jsmntok_t *t = &tokens[i];

        // Should never reach uninitialized tokens
        // assert(t->start != -1 && t->end != -1);

        if (t->type == JSMN_ARRAY || t->type == JSMN_OBJECT){
            j += t->size;
            childSize = t->size;
        }

        slog(LVL_NOISY,DEBUG,"state:%s j:%d i:%d t->size:%d inheritCnt:%d parentCount:%d",
            stateString[state],j,i, t->size,inheritLoop,parentKeyCount);

        if(inheritLoop<=0){
            if(inheritKey){
                free(inheritKey);
                inheritKey=NULL;
            }
        } else { 
          inheritLoop--; 
        }

        if(parentKeyCount<=0){ 
           parentKey=NULL; 
        } else { 
           parentKeyCount--; 
        }
      

        switch (state)
        {
            case START:
                if (t->type != JSMN_OBJECT){
                    slog(LVL_NOISY,DEBUG,"Invalid response: root element must be an object.");
                    break;
                }

                state = KEY;
                object_tokens = t->size;

                if (object_tokens == 0)
                    state = STOP;

                if (object_tokens % 2 != 0)
                    slog(LVL_NOISY,DEBUG,"Invalid response: object must have even number of children.");

                break;

            case KEY:

                if (t->type == JSMN_OBJECT){
                    asprintf(&inheritKey,"%d",arrayCount);
                    // TODO : arrayCount must be per Object, not global
                    arrayCount++;
                    inheritLoop=t->size;
                    parentKeyCount+=inheritLoop;
                    object_tokens+=inheritLoop;
                    slog(LVL_NOISY,DEBUG,"Set inherit key to %s for %d loops",inheritKey,inheritLoop);
                    slog(LVL_NOISY,DEBUG,"Parentkey count reset to %d tokens",parentKeyCount);
                    break;
                } else 
                if (t->type != JSMN_STRING){
                    slog(LVL_NOISY,DEBUG,"Invalid response: object keys must be string or object. (is type %d)",t->type);
                    break;
                }

                state = SKIP;

                currentKey = json_token_tostr(js, t);

                if(inheritKey != NULL){
                    if(parentKey != NULL){
                        asprintf(&storeKey,"%s.%s.%s",parentKey,inheritKey, currentKey);
                    } else {
                        asprintf(&storeKey,"%s.%s",inheritKey,currentKey);
                    }
                } else if(parentKey != NULL) {
                    if(childSize >0){
                        asprintf(&storeKey,"%s.%s", parentKey,currentKey);
                    }
                    childSize-=2;
                    object_tokens++;
                }else{
                    asprintf(&storeKey,"%s", currentKey);
                    object_tokens--;
                }
                state = PRINT;
                break;

            case SKIP:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE){
                    slog(LVL_NOISY,DEBUG,"Invalid response: object values must be strings or primitives.");
                    break;
                }

                object_tokens--;
                state = KEY;

                if (object_tokens == 0)
                    state = STOP;

                break;

            case PRINT:

                str = json_token_tostr(js, t);

                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE){
                    if (t->type == JSMN_ARRAY || t->type == JSMN_OBJECT){
                        if( t->size >0 ){
                            parentKey = currentKey;
                            parentKeyCount = t->size;
                            slog(LVL_NOISY,DEBUG,"Parentkey set to %s for the next %d tokens",parentKey,parentKeyCount);
                        } else {
                            slog(LVL_NOISY,DEBUG,"Parentkey not set, substructure empty");
                            object_tokens++;
                        }
                    } else {
                        slog(LVL_NOISY,DEBUG,"Invalid response: object values must be strings or primitives.");
                        break;
                    }
                } else {
                    slog(LVL_NOISY,DEBUG, "Seting key number %d : %s=%s",storeCount, storeKey,str);
                    //if(!map_haskey(map,storeKey)){
                        ht_insert_simple(map,storeKey,str);
                        storeCount++;
                    //} else {
                    //    slog(LVL_NOISY,DEBUG, "Key is already set and key overwrite is disabled!");
                    //}
                    if(storeKey)
                      free(storeKey);
                }

                object_tokens--;
                state = KEY;
                if (object_tokens == 0){
                    state = STOP;
                }

                break;

            case STOP:
                slog(LVL_NOISY,DEBUG,"Consuming json token : %s", json_token_tostr(js, t));
                // Just consume the tokens
                break;

            default:
                slog(LVL_NOISY,DEBUG,"Invalid state %u", state);
        }
    }

    slog(LVL_NOISY,DEBUG,"Read %d json entries",storeCount);
    jsonEntries += storeCount;

    if(storeCount == 0) return false;
    return true;
}
