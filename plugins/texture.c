#include "../lib/plugins.h"
#include "../lib/util.h"
#include "../lib/ui.h"
#include <duktape.h>
#include "duktools.h"
#include <stdbool.h>

#define PLUGIN_SCOPE "texture"

#ifndef WALLY_texture
#define WALLY_texture

duk_context *ctx = NULL;
pluginHandler *ph;

typedef struct texturePlugin
{
    const texInfo *TI;
    const SDL_Rect *rect;
} texturePlugin;

extern const duk_function_list_entry myMethods[];

char *cleanupPlugin(void *p){
    slog(LVL_NOISY,DEBUG,"Plugin "PLUGIN_SCOPE" uninitialized");
    return NULL;
}

static duk_ret_t js_setter(duk_context *ctx) {
    slog(LVL_INFO,INFO,"Setter called");
    return 0;
}
static duk_ret_t js_getter(duk_context *ctx) {
    slog(LVL_INFO,INFO,"Getter called");
    return 0;
}
// the destructor
duk_ret_t js_texture_dtor(duk_context *ctx)
{
    // The object to delete is passed as first argument instead
    duk_get_prop_string(ctx, 0, "\xff""\xff""deleted");

    bool deleted = duk_to_boolean(ctx, -1);
    duk_pop(ctx);

    // Get the pointer and free
//    if (!deleted) {
//        duk_get_prop_string(ctx, 0, "\xff""\xff""data");
//        texturePlugin *TP = duk_to_pointer(ctx, -1);
//        free(TP->rect);
//        free(TP);
//        duk_pop(ctx);

        // Mark as deleted
//        duk_push_boolean(ctx, true);
//        duk_put_prop_string(ctx, 0, "\xff""\xff""deleted");
//    }
    return 0;
}
#define round(x) ((x) < LONG_MIN-0.5 || (x) > LONG_MAX+0.5 ? error() : ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
// Constructor of the JS Object
duk_ret_t js_texture_ctor(duk_context *ctx)
{
    int ret = 0;
    printf("Creating new "PLUGIN_SCOPE" object");
    slog(LVL_QUIET,INFO, "Creating new "PLUGIN_SCOPE" object");

//    texturePlugin *TP = malloc(sizeof(texturePlugin));
//    TP->rect = malloc(sizeof(SDL_Rect));
//    const char *name = (char*)duk_require_string(ctx, 0);
//    const char *z = duk_safe_to_string(ctx, 1);
//    const char *x = duk_safe_to_string(ctx, 2);
//    const char *y = duk_safe_to_string(ctx, 3);
//    const char *w = duk_safe_to_string(ctx, 4);
//    const char *h = duk_safe_to_string(ctx, 5);
//    const char *c = duk_safe_to_string(ctx, 6);
//    char *cs;
//    // TODO : - if not existent create a hash_map
//    //        - store structure to a hash_map('name');
//    //          so that it can be reached from JS and C
//    asprintf(&cs,"%s %s %s %s %s %s %s",name, z, x, y, w, h, c);
//    slog(LVL_INFO,INFO,"screen::createTexture(%s)",cs);
//    callEx("screen::createTexture",&ret, cs, true, true);
////    free(cs);
//    TP->TI = getTexture(name); 
//     // Push special this binding to the function being constructed
    duk_push_this(ctx);
    // Store the underlying object
    duk_idx_t obj_idx = duk_push_object(ctx);
//    DUK_PUSH_PROP_INT("x",TP->TI->rect->x);
//    DUK_PUSH_PROP_INT("y",TP->TI->rect->y);
//    DUK_PUSH_PROP_INT("w",TP->TI->rect->w);
//    DUK_PUSH_PROP_INT("h",TP->TI->rect->h);
//    DUK_PUSH_PROP_INT("z",TP->TI->z);
//    DUK_PUSH_PROP_INT("color",TP->TI->c);
//    DUK_PUSH_PROP_STRING("name",TP->TI->name);
//    DUK_PUSH_PROP_POINTER("\xff""\xff""data",TP);
//    DUK_PUSH_PROP_BOOL("\xff""\xff""deleted",false);

    //duk_push_pointer(ctx, TI);
    //duk_put_prop_string(ctx, -2, "\xff""\xff""data");
   
    // Store a boolean flag to mark the object as deleted because the destructor may be called several times
    //duk_push_boolean(ctx, false);
    //duk_put_prop_string(ctx, -2, "\xff""\xff""deleted");

    // Store the function destructor
    duk_push_c_function(ctx, js_texture_dtor, 1);
    duk_set_finalizer(ctx, obj_idx);

    return 1;
}

const duk_function_list_entry textureMethods[] = {
    { NULL,    NULL,            0 }
};

char *initPlugin(pluginHandler *_ph){
    slog(LVL_NOISY,FULLDEBUG,"Plugin "PLUGIN_SCOPE" initializing.");
    ph=_ph;
    ctx = ph->ctx;
    duk_push_c_function(ctx, js_texture_ctor, 7 );
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, textureMethods);
    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "Texture");  /* -> stack: [ ] */

    return PLUGIN_SCOPE;
}


#endif
