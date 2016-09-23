#include "../lib/plugins.h"
#include "../lib/util.h"
#include "autoversion.h"
#include <duktape.h>
#include "duktools.h"
#include "dschema.h"
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#define PLUGIN_SCOPE "js"

#ifndef WALLY_DUKTEST
#define WALLY_DUKTEST

duk_context *ctx = NULL;

extern pluginHandler *ph;

extern int setDebug(int lvl);
int registerFunctions(void);
const duk_function_list_entry wallyMethods[];

int js_writeFileSync(duk_context *ctx) {
   unsigned long len;
   void *buf = NULL;
   const char *filename;

   dschema_check(ctx, (const duv_schema_entry[]){
       {"file", duk_is_string},
       {"data", dschema_is_data},
       {NULL}
   });

   filename = duk_get_string(ctx, 0);

   if (duk_is_string(ctx, 1)) {
     buf = (char*) duk_get_lstring(ctx, 1, &len);
   } else {
     buf = duk_get_buffer(ctx, 1, &len);
   }
   slog(TRACE,LOG_JS,"Ready to write out %u bytes to %s",len,filename);
   FILE * out = fopen(filename,"w");
   // TODO : push back error msg into duktape
   if(!out) {
      slog(ERROR,LOG_JS,"Could not write to %s",filename);
      return 1;
   }
   long written = fwrite(buf,1,len,out);
   if(written != len)
   {
      slog(ERROR,LOG_JS,"Could only write %d of %d bytes to %s",written,len,filename);
      return 1;
   }
   fclose(out);

   return 0;
}

int js_evalFile(duk_context *ctx) {
   const char *filename = duk_to_string(ctx, 0);
   slog(DEBUG,LOG_JS,"Execute file : %s (Top : %d)", filename, (long) duk_get_top(ctx));
   duk_eval_file(ctx, filename);
   duk_pop(ctx);
   return 0;
}

int js_commitTransaction(duk_context *ctx) {
   int ret;
   slog(DEBUG,LOG_PLUGIN,"Commit a transaction.");
   callWtx(NULL,NULL);
   ph->transaction = false;
   ph->wtx->elements=0;
   return 0;
}

int js_startTransaction(duk_context *ctx) {
   int ret;
   slog(DEBUG,LOG_PLUGIN,"Started a new transaction. Clearing WTX.");
   ph->transaction = true;
   ph->wtx->elements=0;
   return 0;
}

int js_render(duk_context *ctx) {
   int ret;
   const char *texName = duk_to_string(ctx, 0);
   callWtx("screen::render",(char *)texName);
   return 0;
}

int js_setAutoRender(duk_context *ctx) {
   int ret;
   const bool sar = duk_to_boolean(ctx, 0);
   if(sar == true){
      callWithString("screen::setAutoRender",&ret,"true");
   }else{
      callWithString("screen::setAutoRender",&ret,"false");
   }
   return 0;
}

static int js_getrss(duk_context *ctx) {
	int rss=getCurrentRSS();
	duk_push_int(ctx,rss);
	return 1;
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
    callWithString("screen::showTextureTestScreen",&ret,NULL);
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
    callWtx("screen::setImageScaled",cs);
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
    callWtx("screen::setTextUTF8 ",cs);
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
    callWtx("screen::setText",cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}
 
int js_log(duk_context *ctx)
{
    // screen::setText bauch stampColor stampfont 20 0 Wally TV Test Screen
    int n = duk_get_top(ctx);
    const char *text = strdup(duk_to_string(ctx,0));
    int ret;
    callWtx("screen::log",(char*)text);
    // TODO : free
    //free(text);
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
   time_t t;
   time(&t);
   duk_idx_t obj_idx = duk_push_object(ctx);
   DUK_PUSH_PROP_INT("release",BUILD_NUMBER);
   DUK_PUSH_PROP_STRING("builddate",BUILD_DATE);
   DUK_PUSH_PROP_INT("debug",ph->loglevel);
   DUK_PUSH_PROP_INT("width",ph->width);
   DUK_PUSH_PROP_INT("height",ph->height);
   DUK_PUSH_PROP_INT("uptime",(int)t)
   DUK_PUSH_PROP_STRING("basedir",ph->basedir);
   DUK_PUSH_PROP_STRING("location",ph->location);
   DUK_PUSH_PROP_STRING("uuid",ph->uuid);
   DUK_PUSH_PROP_STRING("cmdlinefile", DEFAULT_CMDLINE_TXT);
   DUK_PUSH_PROP_STRING("configfile", DEFAULT_CONFIG_TXT);
   DUK_PUSH_PROP_STRING("ubootenv", DEFAULT_UBOOT_ENV);
   DUK_PUSH_PROP_STRING("flagfile", FLAGFILE);
   DUK_PUSH_PROP_STRING("kernelconf" , KERNEL_DEFAULT_CONFIG);
#ifdef __powerpc__
   DUK_PUSH_PROP_STRING("arch" , "ppc");
#endif
#ifdef __mips__
   DUK_PUSH_PROP_STRING("arch" , "mips");
#endif
#ifdef __i386__
   DUK_PUSH_PROP_STRING("arch" , "x86_32");
#endif
#ifdef __amd64__
   DUK_PUSH_PROP_STRING("arch" , "x86_64");
#endif
#ifdef __ARM_ARCH_6__
      DUK_PUSH_PROP_STRING("arch" , "armv6");
#endif
#ifdef __ARM_ARCH_7__
         DUK_PUSH_PROP_STRING("arch" , "armv7");
#endif
#ifdef __aarch64__
	    DUK_PUSH_PROP_STRING("arch" , "arm64");
#endif
   DUK_PUSH_PROP_BOOL("registered", ph->registered);
   DUK_PUSH_PROP_BOOL("ssdp", ph->ssdp);
   DUK_PUSH_PROP_BOOL("cloud", ph->cloud);
// TODO : export configMap
//   int kc=0,i;
//   char **keys = ht_keys(ph->configMap, &kc);
//   slog(TRACE,ERROR,"Exporting %d keys to JS",kc);
//   for(int i=0; i < key_count; i++){
//      char *name = keys[i];
//      DUK_PUSH_PROP_STRING(name, ht_get_simple(ph->configMap, name));
//   }

   return 1;
}

int js_setDebug(duk_context *ctx)
{
    int n = duk_get_top(ctx);
    int lvl = duk_to_int(ctx,0);
    slog(TRACE,LOG_JS,"Set loglevel to : %d",lvl);
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
    callWtx("screen::loadFont",cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}


int js_createColor(duk_context *ctx)
{
    // screen::createColor name RGB alpha
    int n = duk_get_top(ctx);  /* #args */
    const char *name = duk_to_string(ctx,0);
    const char *RGB = duk_to_string(ctx,1);
    const char *A = duk_to_string(ctx,2);
    char *cs;
    int ret;
    asprintf(&cs,"%s %s %s",name,RGB,A);
    callWtx("screen::createColor",cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}

int js_destroyTexture(duk_context *ctx)
{
    // screen::destroyTexture name
    int n = duk_get_top(ctx);  /* #args */
    const char *name = duk_to_string(ctx,0);
    callWtx("screen::destroyTexture",(char*)name);
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
    //newSimpleWtx(&wtx,"screen::createTexture",cs);
    callWtx("screen::createTexture",cs);
    free(cs);
    return 1;
}

int evalFile(void *file){
    if (duk_peval_file(ctx,file) != 0) {
        slog(ERROR,LOG_JS,"JSError in file %s : %s", file, duk_safe_to_string(ctx, -1));
    } else {
        slog(DEBUG,LOG_JS,"result is: %s", duk_safe_to_string(ctx, -1));
    }
    return 0;
}

duk_ret_t js_exec(duk_context *ctx){
   int ret;
   char *str,*cmd;
   char *tofree = str = strdup(duk_to_string(ctx,0));
   assert(str != NULL);
   cmd = strsep(&str, " \t");
   slog(DEBUG,LOG_JS,"call : %s(%s)", cmd,str);
   callWtx(cmd,str);
   free(tofree);
   return 0;
}

int js_evalScript(duk_context *ctx){
    //duk_push_string(ctx, str);
    if (duk_peval(ctx) != 0) {
        slog(ERROR,LOG_JS,"JSEval : %s", duk_safe_to_string(ctx, -1));
    } else {
        slog(DEBUG,LOG_JS,"result is: %s", duk_safe_to_string(ctx, -1));
    }
    return 1;
}

int evalScript(void *str){
    duk_push_string(ctx, str);
    if (duk_peval(ctx) != 0) {
        slog(ERROR,LOG_JS,"JSError : %s", duk_safe_to_string(ctx, -1));
    } else {
        slog(DEBUG,LOG_JS,"result is: %s", duk_safe_to_string(ctx, -1));
    }
    return 1;
}

char *cleanupPlugin(void *p){
    // TO BE DONE somewhere else : duk_destroy_heap(ctx);
    slog(DEBUG,LOG_PLUGIN,"Plugin "PLUGIN_SCOPE" uninitialized");
    return NULL;
}

duk_ret_t js_wally_ctor(duk_context *ctx)
{
    slog(DEBUG,LOG_JS, "Getting access to THE wally object.");

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
    { "eval",                 js_evalScript, 1 },
    { "exec",                 js_exec, 1 },
    { "render",               js_render, 1 },
    { "setAutoRender",        js_setAutoRender, 1 },
    { "getrss",		      js_getrss, 0},
    { "startTransaction",     js_startTransaction, 0},
    { "commitTransaction",    js_commitTransaction, 0},
    { "writeFileSync",        js_writeFileSync, 2},
    { NULL,                   NULL, 0 }
};

//const function_list_entry c_JSMethods[] = {
//   {    PLUGIN_SCOPE"::eval"	 ,WFUNC_SYNC, evalScript, 0 },
//   { 	PLUGIN_SCOPE"::evalFile" ,WFUNC_SYNC, evalFile,   0 },
//   {	NULL, 0, NULL, 0 }
//};

char *initPlugin(pluginHandler *_ph){
   ph=_ph;
   slog(DEBUG,LOG_PLUGIN,"Plugin "PLUGIN_SCOPE" initializing (PH: %p)",ph);
   ctx = ph->ctx;

   slog(TRACE,LOG_JS,"Constructing wally object");

   wally_put_function(PLUGIN_SCOPE"::evalFile" ,WFUNC_SYNC, &evalFile,   0);
   wally_put_function(PLUGIN_SCOPE"::eval"     ,WFUNC_SYNC, &evalScript, 0);
   
   duk_push_c_function(ctx, js_wally_ctor, 0 );
   duk_push_object(ctx);
   duk_put_function_list(ctx, -1, wallyMethods);
   duk_put_prop_string(ctx, -2, "prototype");
   duk_put_global_string(ctx, "Wally");  /* -> stack: [ ] */

    slog(TRACE,LOG_PLUGIN,"Plugin initialized. PH is at 0x%x",ph);
    return PLUGIN_SCOPE;
}

#endif
