/*
   the watchdog will then display a message/image in a texture for a certain time
   you can push an item of type messages or image into the watchdog queue
   you can make the item blink
   you can remove the item
*/

#include "../lib/plugins.h"
#include "../lib/util.h"
#include "../lib/ui.h"
#include <duv/duktape.h>
#include <stdbool.h>

#define PLUGIN_SCOPE "watchdog"
#define JS_SCOPE     "Watchdog"

#ifndef WALLY_watchdog
#define WALLY_watchdog

duk_context *ctx = NULL;
extern pluginHandler *ph;

extern const duk_function_list_entry myMethods[];

char *cleanupPlugin(void *p){
    slog(LVL_NOISY,DEBUG,"Plugin "PLUGIN_SCOPE" uninitialized");
    return NULL;
}

int watchdog_pushImage(char *args){
   int timeout=5;
   const char *name    = NULL ;//= args[0];
   const char *texture = NULL ;//= args[1];
   const char *type    = NULL ;//= args[2];
   const char *img     = NULL ;//= args[4];
   //getNum(args[3],&timeout);
   if(timeout == 0 || !texture || !type || !img){
      slog(LVL_QUIET,ERROR,"Usage : "PLUGIN_SCOPE"::pushImage name texture type timeout imagePath");
      return 0;
   }
   slog(LVL_NOISY,DEBUG,"Push-%s(%s,%s,%d,%s)",type,texture,timeout,img);
   return 0;
}

int js_watchdog_blink(duk_context *ctx){
   return 0;
}

int js_watchdog_remove(duk_context *ctx){
   return 0;
}

int watchdog_blink(char *name){
   return 0;
}

int watchdog_remove(char *name){
   return 0;
}

int watchdog_push(char *args){
   int timeout=5;
   const char *name    = NULL ;//= args[0];
   const char *texture = NULL ;//= args[1];
   const char *type    = NULL ;//= args[2];
   const char *message = NULL ;//= args[4];
   //getNum(args[3],&timeout);
   if(timeout == 0 || !texture || !type || !message){
      slog(LVL_QUIET,ERROR,"Usage : "PLUGIN_SCOPE"::push name texture type timeout message");
      return 0;
   }
   slog(LVL_NOISY,DEBUG,"Push-%s(%s,%d,%s)",type,texture,timeout,message);
   return 0;
}

duk_ret_t js_watchdog_push(duk_context *ctx){
   slog(LVL_NOISY,DEBUG,"Push JS not implemented");
   return 0;
}

// the destructor
duk_ret_t js_watchdog_dtor(duk_context *ctx)
{
    // The object to delete is passed as first argument instead
    duk_get_prop_string(ctx, 0, "\xff""\xff""deleted");

    bool deleted = duk_to_boolean(ctx, -1);
    duk_pop(ctx);

    // Get the pointer and free
    if (!deleted) {
        duk_get_prop_string(ctx, 0, "\xff""\xff""data");
        void *freePtr = duk_to_pointer(ctx, -1);
        free(freePtr);
        duk_pop(ctx);

        // Mark as deleted
        duk_push_boolean(ctx, true);
        duk_put_prop_string(ctx, 0, "\xff""\xff""deleted");
    }
    return 0;
}

// Constructor of the JS Object
duk_ret_t js_watchdog_ctor(duk_context *ctx)
{
    void *ptr = NULL;
    slog(LVL_NOISY,DEBUG, "Creating new object of "PLUGIN_SCOPE);

    // Push special this binding to the function being constructed
    duk_push_this(ctx);

    // Store the underlying object
    duk_push_pointer(ctx, ptr);
    duk_put_prop_string(ctx, -2, "\xff""\xff""data");

    // TODO : - if not existand create a hash_map
    //        - store structure to a hash_map('name');
    //          so that it can be reached from JS and C
    
    // Store a boolean flag to mark the object as deleted because the destructor may be called several times
    duk_push_boolean(ctx, false);
    duk_put_prop_string(ctx, -2, "\xff""\xff""deleted");

    // Store the function destructor
    duk_push_c_function(ctx, js_watchdog_dtor, 1);
    duk_set_finalizer(ctx, -2);

    return 0;
}

duk_ret_t js_watchdog_toString(duk_context *ctx)
{
   duk_push_this(ctx);  /* -> stack: [ this ] */
   duk_get_prop_string(ctx, 0, "\xff""\xff""data");
   return 1;
}

const duk_function_list_entry watchdogMethods[] = {
    { "toString",   js_watchdog_toString, 0  },
    { "push",       js_watchdog_push,     5  },
    { "remove",     js_watchdog_remove,   1  },
    { "blink",      js_watchdog_blink,    1  },
    { NULL,    NULL,            0 }
};

const function_list_entry c_watchdogMethods[] = {
    { PLUGIN_SCOPE"::push",       WFUNC_SYNC, watchdog_push, 5},
    { PLUGIN_SCOPE"::pushImage",  WFUNC_SYNC, watchdog_pushImage, 5},
    { PLUGIN_SCOPE"::remove",     WFUNC_SYNC, watchdog_remove, 1},
    { PLUGIN_SCOPE"::blink",      WFUNC_SYNC, watchdog_blink,1},
    { NULL, 0, NULL, 0 }
};


char *initPlugin(pluginHandler *_ph){
    slog(LVL_NOISY,DEBUG,"Plugin "PLUGIN_SCOPE" initializing.");
    ph=_ph;
    ctx = ph->ctx;

    wally_put_function_list(c_watchdogMethods);

    duk_push_c_function(ctx, js_watchdog_ctor, 7 );
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, watchdogMethods);
    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, JS_SCOPE);

    return PLUGIN_SCOPE;
}


#endif
