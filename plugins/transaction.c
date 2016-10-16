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

#define PLUGIN_SCOPE "transaction"

duk_context *ctx = NULL;

extern pluginHandler *ph;

int js_commitTransaction(duk_context *ctx) {
   dschema_check(ctx, (const duv_schema_entry[]){
       {"name", duk_is_string},
       {NULL}
   });
   const char *name = duk_get_string(ctx, 0);
   slog(DEBUG,LOG_PLUGIN,"Commiting and unlocking transaction %s at 0x%x",name,ph->wtx);
   callWtx(NULL,NULL);
   pthread_mutex_unlock(&ph->taMutex);
   return 1;
}

int js_newTransaction(duk_context *ctx) {
   wally_call_ctx* wtx = NULL;
   dschema_check(ctx, (const duv_schema_entry[]){
       {"name", duk_is_string},
       {NULL}
   });
   const char *name = duk_get_string(ctx, 0);
   slog(DEBUG,LOG_PLUGIN,"New transaction %s created",name);
   initWtx(&wtx);
   pushSimpleWtx(&wtx,"nop","nop");
   ht_insert_simple(ph->transactions,name,wtx);
   slog(DEBUG,LOG_PLUGIN,"New wtx 0x%x created",wtx);
   return 1; 
}

int js_startTransaction(duk_context *ctx) {
   dschema_check(ctx, (const duv_schema_entry[]){
       {"name", duk_is_string},
       {NULL}
   });
   const char *name = duk_get_string(ctx, 0);
   slog(DEBUG,LOG_PLUGIN,"Set transaction %s",name);
   pthread_mutex_lock(&ph->taMutex);
   ph->wtx = ht_get_simple(ph->transactions,name); 
   slog(DEBUG,LOG_PLUGIN,"Set and locked current transaction to 0x%x",ph->wtx);
   return 1; 
}

char *cleanupPlugin(void *p){
    // TODO : free all open TA's
    slog(DEBUG,LOG_PLUGIN,"Plugin "PLUGIN_SCOPE" uninitialized");
    return NULL;
}

duk_ret_t transaction_ctor(duk_context *ctx)
{
    slog(DEBUG,LOG_JS, "Getting access to the transaction object.");

    duk_push_this(ctx);
    duk_dup(ctx, 0);  /* -> stack: [ name this name ] */
    duk_put_prop_string(ctx, -2, "name");  /* -> stack: [ name this ] */
//  TODO : initialize maps, vitual object, etc
    return 1;
}

const duk_function_list_entry taMethods[] = {
    { "newTransaction",       js_newTransaction, 1},
    { "startTransaction",     js_startTransaction, 1},
    { "commitTransaction",    js_commitTransaction, 1},
    { NULL,                   NULL, 0 }
};

char *initPlugin(pluginHandler *_ph){
   ph=_ph;
   slog(DEBUG,LOG_PLUGIN,"Plugin "PLUGIN_SCOPE" initializing (PH: %p)",ph);
   ctx = ph->ctx;

   slog(TRACE,LOG_JS,"Constructing transaction object");

   duk_push_c_function(ctx, transaction_ctor, 0 );
   duk_push_object(ctx);
   duk_put_function_list(ctx, -1, taMethods);
   duk_put_prop_string(ctx, -2, "prototype");
   duk_put_global_string(ctx, "Transaction");  /* -> stack: [ ] */

    slog(TRACE,LOG_PLUGIN,"Plugin initialized. PH is at 0x%x",ph);
    return PLUGIN_SCOPE;
}
