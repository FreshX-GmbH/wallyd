#include "../lib/plugins.h"
#include "../lib/util.h"
#include <duktape.h>
#include <stdbool.h>

#define PLUGIN_SCOPE "myPlugin"

#ifndef WALLY_MYPLUGIN
#define WALLY_MYPLUGIN

duk_context *ctx = NULL;
pluginHandler *ph;

typedef struct myPluginStructure
{
    const char *name;
} myPluginStructure;

extern const duk_function_list_entry myPluginMethods[];

char *cleanupPlugin(void *p){
    slog(DEBUG,DEBUG,"Plugin "PLUGIN_SCOPE" uninitialized");
    return NULL;
}

// the destructor
duk_ret_t js_myPlugin_dtor(duk_context *ctx)
{
    // The object to delete is passed as first argument instead
    duk_get_prop_string(ctx, 0, "\xff""\xff""deleted");

    bool deleted = duk_to_boolean(ctx, -1);
    duk_pop(ctx);

    // Get the pointer and free it
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
duk_ret_t js_myPlugin_ctor(duk_context *ctx)
{
    slog(DEBUG,DEBUG, "Creating new object of "PLUGIN_SCOPE);

    myPluginStructure *mps = malloc(sizeof(myPluginStructure));
    mps->name = duk_require_string(ctx, 0);
    
    // Push special this binding to the function being constructed
    duk_push_this(ctx);

    // Store the underlying object
    duk_push_pointer(ctx, mps);
    duk_put_prop_string(ctx, -2, "\xff""\xff""data");

    // TODO : - if not existand create a hash_map
    //        - store structure to a hash_map('name');
    //          so that it can be reached from JS and C
    
    // Store a boolean flag to mark the object as deleted because the destructor may be called several times
    duk_push_boolean(ctx, false);
    duk_put_prop_string(ctx, -2, "\xff""\xff""deleted");

    // Store the function destructor
    duk_push_c_function(ctx, js_myPlugin_dtor, 1);
    duk_set_finalizer(ctx, -2);

    return 0;
}

int myPluginInfo(char *i){
   if(i) {
      slog(DEBUG,DEBUG,"Info : %s", i);
      // TODO : Get structure from hashmap('name');
      return true;
   } else {
      slog(DEBUG,DEBUG,"Wrong parameters calling "PLUGIN_SCOPE"::info <name>");
      return false;
   }
}

duk_ret_t js_myPlugin_toString(duk_context *ctx)
{
   duk_push_this(ctx);  /* -> stack: [ this ] */
   duk_get_prop_string(ctx, 0, "\xff""\xff""data");
   myPluginStructure *mps = duk_to_pointer(ctx, -1);
   duk_pop(ctx);
   duk_push_sprintf(ctx, "%s",mps->name);
   return 1;
}

duk_ret_t js_myPlugin_info(duk_context *ctx)
{
   duk_push_this(ctx);  /* -> stack: [ this ] */
   duk_get_prop_string(ctx, 0, "\xff""\xff""data");
   myPluginStructure *mps = duk_to_pointer(ctx, -1);
   duk_pop(ctx);
   duk_push_sprintf(ctx, "{ name : %s }",mps->name);
   return 1;
}

const duk_function_list_entry myPluginMethods[] = {
    { "info",       js_myPlugin_info,      0   },
    { "toString",   js_myPlugin_toString,  0   },
    { NULL,    NULL,            0 }
};

const function_list_entry c_myPluginMethods[] = {
    { PLUGIN_SCOPE"::info", WFUNC_SYNC, myPluginInfo, 0 },
    { NULL, 0, NULL, 0 }
};


char *initPlugin(pluginHandler *_ph){
    slog(DEBUG,FULLDEBUG,"Plugin "PLUGIN_SCOPE" initializing.");
    ph=_ph;
    ctx = ph->ctx;

    duk_push_c_function(ctx, js_myPlugin_ctor, 1 );
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, myPluginMethods);
    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "myPlugin");  /* -> stack: [ ] */

    wally_put_function_list(ph,c_myPluginMethods);
    return PLUGIN_SCOPE;
}


#endif
