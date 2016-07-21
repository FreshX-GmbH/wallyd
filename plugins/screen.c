#include "../lib/plugins.h"
#include "../lib/util.h"
#include "../lib/ui.h"
#include <duv/duktape.h>
#include "duktools.h"
#include <stdbool.h>

#define PLUGIN_SCOPE "screen"

#ifndef WALLY_screen
#define WALLY_screen

duk_context *ctx = NULL;
pluginHandler *ph;

typedef struct screenPlugin
{
} screenPlugin;

extern const duk_function_list_entry myMethods[];

char *cleanupPlugin(void *p){
    slog(LVL_NOISY,DEBUG,"Plugin "PLUGIN_SCOPE" uninitialized");
    return NULL;
}

duk_ret_t getFonts(duk_context *ctx){
    duk_push_string(ctx, "[a,b,c]");
    return 1;
}

duk_ret_t getColors(duk_context *ctx){
    duk_push_string(ctx, "[a,b,c]");
    return 1;
}
duk_ret_t getTextures(duk_context *ctx){
    duk_push_string(ctx, "[a,b,c]");
    return 1;
}

//duk_ret_t setAutoRender(duk_context *ctx){
//    slog(LVL_NOISY,DEBUG,"Set autorender to ...");
//    return 0;
//}


// the destructor
duk_ret_t js_screen_dtor(duk_context *ctx)
{
    // The object to delete is passed as first argument instead
    duk_get_prop_string(ctx, 0, "\xff""\xff""deleted");

    bool deleted = duk_to_boolean(ctx, -1);
    duk_pop(ctx);

    // Get the pointer and free
    if (!deleted) {
        duk_get_prop_string(ctx, 0, "\xff""\xff""data");
        screenPlugin *SP = duk_to_pointer(ctx, -1);
        free(SP);
        duk_pop(ctx);

        // Mark as deleted
        duk_push_boolean(ctx, true);
        duk_put_prop_string(ctx, 0, "\xff""\xff""deleted");
    }
    return 0;
}
// Constructor of the JS Object
duk_ret_t js_screen_ctor(duk_context *ctx)
{
    int ret = 0;
    slog(LVL_NOISY,DEBUG, "Creating new "PLUGIN_SCOPE" object");

    screenPlugin *SP = malloc(sizeof(screenPlugin));
    duk_push_this(ctx);
    // Store the underlying object
    duk_idx_t obj_idx = duk_push_object(ctx);
    //DUK_PUSH_PROP_STRING("textures",getTextureList());
    //DUK_PUSH_PROP_STRING("colors",getTextureList());
    //DUK_PUSH_PROP_STRING("fonts",getTextureList());
    //DUK_PUSH_PROP_STRING("name",TP->TI->name);
    DUK_PUSH_PROP_POINTER("\xff""\xff""data",SP);
    DUK_PUSH_PROP_BOOL("\xff""\xff""deleted",false);

    // Store the function destructor
    duk_push_c_function(ctx, js_screen_dtor, 1);
    duk_set_finalizer(ctx, obj_idx);

    return 1;
}

const duk_function_list_entry screenMethods[] = {
    { "getColors",      getColors,   0   },
    { "getFonts",       getFonts,    0   },
    { "getTextures",    getTextures, 0   },
//    { "setAutoRender",  setAutoRender, 1 },
//    { "render",         js_render, 1 },
    { NULL,    NULL,            0 }
};

char *initPlugin(pluginHandler *_ph){
    slog(LVL_NOISY,FULLDEBUG,"Plugin "PLUGIN_SCOPE" initializing.");
    ph=_ph;
    ctx = ph->ctx;

    duk_push_c_function(ctx, js_screen_ctor, 7 );
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, screenMethods);
    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "screen");  /* -> stack: [ ] */

    return PLUGIN_SCOPE;
}


#endif
