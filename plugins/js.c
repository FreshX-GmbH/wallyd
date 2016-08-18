#include "../lib/plugins.h"
#include "../lib/util.h"
#include "autoversion.h"
#include <duktape.h>
#include "duktools.h"
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#define PLUGIN_SCOPE "js"

#ifndef WALLY_DUKTEST
#define WALLY_DUKTEST

duk_context *ctx = NULL;

pluginHandler *ph;

extern int setDebug(int lvl);
extern int showTextureTestScreen(char *);
int registerFunctions(void);
const duk_function_list_entry wallyMethods[];

static int js_evalFile(duk_context *ctx) {
   const char *filename = duk_to_string(ctx, 0);
   slog(LVL_NOISY,DEBUG,"Execute file : %s (Top : %d)", filename, (long) duk_get_top(ctx));
   duk_eval_file(ctx, filename);
   duk_pop(ctx);
   return 0;
}

static int js_render(duk_context *ctx) {
   int ret;
   const char *texName = duk_to_string(ctx, 0);
   callWithString("screen::render",&ret,texName);
   return 0;
}

static int js_setAutoRender(duk_context *ctx) {
   int ret;
   const bool sar = duk_to_boolean(ctx, 0);
   if(sar == true){
      callWithString("screen::setAutoRender",&ret,"true");
   }else{
      callWithString("screen::setAutoRender",&ret,"false");
   }
   return 0;
}

static int js_readfile(duk_context *ctx) {
	const char *filename = duk_to_string(ctx, 0);
	FILE *f = NULL;
	long len;
	void *buf;
	size_t got;

	if (!filename) { goto error; }

	f = fopen(filename, "rb");
	if (!f) { goto error; }

	if (fseek(f, 0, SEEK_END) != 0) { goto error; }
	len = ftell(f);
	if (fseek(f, 0, SEEK_SET) != 0) { goto error; }

	buf = duk_push_fixed_buffer(ctx, (size_t) len);

	got = fread(buf, 1, len, f);
	if (got != (size_t) len) { goto error; }

	fclose(f);
	f = NULL;

	return 1;

 error:
	if (f) {
		fclose(f);
	}
	return DUK_RET_ERROR;
}

duk_ret_t js_readdir(duk_context *ctx) {
    const char *dirname = duk_to_string(ctx, 0);

    DIR *dirHandle;
    struct dirent *dirEntry;
    struct stat *entry=malloc(sizeof(struct stat));
    
    duk_push_this(ctx);
    // Store the underlying object
    
    dirHandle = opendir(dirname);
    if (dirHandle) {
        while (0 != (dirEntry = readdir(dirHandle))) {
            stat(dirEntry->d_name,entry);
            duk_idx_t obj_idx = duk_push_object(ctx);
            DUK_PUSH_PROP_INT(dirEntry->d_name,entry->st_mode);
            puts(dirEntry->d_name);
        }
        closedir(dirHandle);
        free(entry);
        return 1;
    } else {
        free(entry);
        return DUK_RET_ERROR;
    }
}


int js_showTextureTestScreen(duk_context *ctx)
{
    int ret;
    call("screen::showTextureTestScreen",&ret,NULL);
    return 1;
}

int js_setImageScaled(duk_context *ctx)
{
    // screen::setImageScaled main images/wally1920x1080.png
    int n = duk_get_top(ctx);
    const char *name = duk_to_string(ctx,0);
    const char *file = duk_to_string(ctx,1);
    int ret;
    char *cs;
    asprintf(&cs,"%s %s",name,file);
    call("screen::setImageScaled",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}

int js_setTextUTF8(duk_context *ctx)
{
    // screen::setText bauch stampColor stampfont 20 0 Wally TV Test Screen
    int n = duk_get_top(ctx);
    const char *name = duk_to_string(ctx,0);
    const char *color = duk_to_string(ctx,1);
    const char *font = duk_to_string(ctx,2);
    const char *x   = duk_to_string(ctx,3);
    const char *y   = duk_to_string(ctx,4);
    const char *txt = duk_to_string(ctx,5);
    int ret;
    char *cs;
    asprintf(&cs,"%s %s %s %s %s %s",name,color,font,x,y,txt);
    call("screen::setTextUTF8 ",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}
 
int js_setText(duk_context *ctx)
{
    // screen::setText bauch stampColor stampfont 20 0 Wally TV Test Screen
    int n = duk_get_top(ctx);
    const char *name = duk_to_string(ctx,0);
    const char *color = duk_to_string(ctx,1);
    const char *font = duk_to_string(ctx,2);
    const char *x   = duk_to_string(ctx,3);
    const char *y   = duk_to_string(ctx,4);
    const char *txt = duk_to_string(ctx,5);
    int ret;
    char *cs;
    asprintf(&cs,"%s %s %s %s %s %s",name,color,font,x,y,txt);
    call("screen::setText",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}
 
int js_log(duk_context *ctx)
{
    // screen::setText bauch stampColor stampfont 20 0 Wally TV Test Screen
    int n = duk_get_top(ctx);
    const char *text = duk_to_string(ctx,0);
    int ret;
//    char *cs = strdup(text);
///    asprintf(&cs,"%s",text);
    callWithString("screen::log",&ret,text);
    // TODO : make copy of the string IN the function utilizing it, not here
 //   free(cs);
    return 1;
}
  
#define DUK_PUSH_PROP_INT(a,b) duk_push_int(ctx, b); duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_UINT(a,b) duk_push_uint(ctx, b); duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_BOOL(a,b) duk_push_boolean(ctx, b); duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_STRING(a,b) duk_push_string(ctx, b);duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_POINTER(a,b) duk_push_pointer(ctx, b);duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_FLOAT(a,b) duk_push_number(ctx, b);duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_DOUBLE(a,b) duk_push_number(ctx, b);duk_put_prop_string(ctx, obj_idx, a);
duk_ret_t js_getConfig(duk_context *ctx)
{
   duk_idx_t obj_idx = duk_push_object(ctx);
   DUK_PUSH_PROP_INT("release",BUILD_NUMBER);
   DUK_PUSH_PROP_INT("builddate",BUILD_DATE);
   DUK_PUSH_PROP_INT("debug",ph->loglevel);
   DUK_PUSH_PROP_INT("width",ph->width);
   DUK_PUSH_PROP_INT("height",ph->height);
   DUK_PUSH_PROP_STRING("basedir",ph->basedir);
   DUK_PUSH_PROP_STRING("location",ph->location);
   DUK_PUSH_PROP_STRING("uuid",ph->uuid);
   DUK_PUSH_PROP_STRING("cmdlinefile", DEFAULT_CMDLINE_TXT);
   DUK_PUSH_PROP_STRING("configfile", DEFAULT_CONFIG_TXT);
   DUK_PUSH_PROP_STRING("ubootenv", DEFAULT_UBOOT_ENV);
   DUK_PUSH_PROP_STRING("flagfile", FLAGFILE);
   DUK_PUSH_PROP_STRING("kernelconf" , KERNEL_DEFAULT_CONFIG);
   DUK_PUSH_PROP_BOOL("registered", ph->registered);
   DUK_PUSH_PROP_BOOL("ssdp", ph->ssdp);
   DUK_PUSH_PROP_BOOL("cloud", ph->cloud);
   return 1;
}

int js_setDebug(duk_context *ctx)
{
    int n = duk_get_top(ctx);
    int lvl = duk_to_int(ctx,0);
    slog(LVL_NOISY,DEBUG,"set debug : %d",lvl);
    ph->loglevel = lvl;
    return 1;
}
 
int js_loadFont(duk_context *ctx)
{
    // screen::loadFont name file size
    int n = duk_get_top(ctx);
    const char *name = duk_to_string(ctx,0);
    const char *file = duk_to_string(ctx,1);
    const char *size = duk_to_string(ctx,2);
    char *cs;
    int ret;
    asprintf(&cs,"%s %s %s",name,file,size);
    call("screen::loadFont",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}


int js_createColor(duk_context *ctx)
{
    // screen::createTexture name RGB alpha
    int n = duk_get_top(ctx);  /* #args */
    const char *name = duk_to_string(ctx,0);
    const char *RGB = duk_to_string(ctx,1);
    const char *A = duk_to_string(ctx,2);
    char *cs;
    int ret;
    asprintf(&cs,"%s %s %s",name,RGB,A);
    call("screen::createColor",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}

int js_destroyTexture(duk_context *ctx)
{
    // screen::destroyTexture name
    int n = duk_get_top(ctx);  /* #args */
    const char *name = duk_to_string(ctx,0);
    char *cs;
    int ret;
    asprintf(&cs,"%s",name);
    call("screen::destroyTexture",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}


int js_createTexture(duk_context *ctx)
{
    // screen::createTexture main 10 0 0 100% -16 FFFFFF
    int n = duk_get_top(ctx);  /* #args */
    const char *name = duk_to_string(ctx,0);
    const char *prio = duk_to_string(ctx,1);
    const char *x = duk_to_string(ctx,2);
    const char *y = duk_to_string(ctx,3);
    const char *w = duk_to_string(ctx,4);
    const char *h = duk_to_string(ctx,5);
    const char *col = duk_to_string(ctx,6);
    char *cs;
    int ret;
    asprintf(&cs,"%s %s %s %s %s %s %s",name,prio,x,y,w,h,col);
    call("screen::createTexture",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it
    //free(cs);
    return 1;
}

int evalFile(char *file){
    if (duk_peval_file(ctx,file) != 0) {
        slog(LVL_QUIET,ERROR,"JSError in file %s : %s", file, duk_safe_to_string(ctx, -1));
    } else {
        slog(LVL_NOISY,DEBUG,"result is: %s", duk_safe_to_string(ctx, -1));
    }
    return 0;
}

duk_ret_t js_exec(duk_context *ctx){
   int ret;
   char *str,*cmd;
   char *tofree = str = strdup(duk_to_string(ctx,0));
   assert(str != NULL);
   cmd = strsep(&str, " \t");
   slog(LVL_NOISY,DEBUG,"call : %s(%s)", cmd,str);
   call(cmd,&ret,str);
   free(tofree);
   return 0;
}


duk_ret_t evalScript(char *str){
    duk_push_string(ctx, str);
    if (duk_peval(ctx) != 0) {
        slog(LVL_QUIET,ERROR,"JSError : %s", duk_safe_to_string(ctx, -1));
    } else {
        slog(LVL_NOISY,DEBUG,"result is: %s", duk_safe_to_string(ctx, -1));
    }
    return 0;
}

char *cleanupPlugin(void *p){
    // TO BE DONE somewhere else : duk_destroy_heap(ctx);
    slog(LVL_NOISY,DEBUG,"Plugin "PLUGIN_SCOPE" uninitialized");
    return NULL;
}

duk_ret_t js_wally_ctor(duk_context *ctx)
{
    slog(LVL_NOISY,DEBUG, "Getting access to THE wally object.");

    duk_push_this(ctx);
    duk_dup(ctx, 0);  /* -> stack: [ name this name ] */
    duk_put_prop_string(ctx, -2, "name");  /* -> stack: [ name this ] */
//  TODO : initialize maps, vitual object, etc
    return 1;
}

const duk_function_list_entry wallyMethods[] = {
    { "showTextureTestScreen",js_showTextureTestScreen, 0 },
    { "createTexture",        js_createTexture, 7 },
    { "createColor",          js_createColor, 3 },
    { "loadFont",             js_loadFont, 3 },
    { "destroyTexture",       js_destroyTexture, 1 },
    { "setImageScaled",       js_setImageScaled, 2 },
    { "setDebug",             js_setDebug, 1 },
    { "setText",              js_setText, 6 },
    { "setTextUTF8",          js_setTextUTF8, 6 },
    { "log",                  js_log, 1 },
    { "getConfig",            js_getConfig, 0 },
    { "readFile",             js_readfile, 1 },
    { "readDir",              js_readdir, 1 },
    { "evalFile",             js_evalFile, 1 },
    { "exec",                 js_exec, 1 },
    { "render",               js_render, 1 },
    { "setAutoRender",        js_setAutoRender, 1 },
    { NULL,                   NULL, 0 }
};

const function_list_entry c_JSMethods[] = {
   {    PLUGIN_SCOPE"::eval"	 ,WFUNC_SYNC, evalScript, 0 },
   { 	PLUGIN_SCOPE"::evalFile" ,WFUNC_SYNC, evalFile,   0 },
   {	NULL, 0, NULL, 0 }
};

char *initPlugin(pluginHandler *_ph){
   slog(LVL_NOISY,FULLDEBUG,"Plugin "PLUGIN_SCOPE" initializing");
   ph=_ph;
   ctx = ph->ctx;

   slog(LVL_ALL,DEBUG,"Constructing wally object");

   wally_put_function_list(c_JSMethods);
   
   duk_push_c_function(ctx, js_wally_ctor, 0 );
   duk_push_object(ctx);
   duk_put_function_list(ctx, -1, wallyMethods);
   duk_put_prop_string(ctx, -2, "prototype");
   duk_put_global_string(ctx, "Wally");  /* -> stack: [ ] */

    slog(LVL_ALL,DEBUG,"Plugin initialized. PH is at 0x%x",_ph);
    return PLUGIN_SCOPE;
}


#endif
