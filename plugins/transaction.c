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

int c_commit(void *p){
   return 0;
}

int lockTransaction(int id){
   pthread_mutex_lock(&ph->taMutex);
   //ph->transaction[id] = id;
   slog(DEBUG,LOG_PLUGIN,"Locked transaction %d at 0x%x",id,ph->transactions[id]);
   return id;
}

int js_lockTransaction(duk_context *ctx) {
   dschema_check(ctx, (const duv_schema_entry[]){
       {"id", duk_is_number},
       {NULL}
   });
   lockTransaction(duk_get_int(ctx, 0));
   return 1;
}


int js_commitTransaction(duk_context *ctx) {
   dschema_check(ctx, (const duv_schema_entry[]){
       {"id", duk_is_number},
       {NULL}
   });
   int id = duk_get_int(ctx, 0);
   slog(DEBUG,LOG_PLUGIN,"Commiting transaction %d at 0x%x",id,ph->transactions[id]);
   commitWtx(id);
   //freeWtx(id);
   //ph->transaction = 0;
   pthread_mutex_unlock(&ph->wtxMutex);
   return 1;
}

int js_newTransaction(duk_context *ctx) {
   wally_call_ctx* wtx = NULL;
   ph->transactionCount=ph->transactionCount+1;
   int id = ph->transactionCount;
   pthread_mutex_lock(&ph->taMutex);
   //int ret = newWtx(id,&wtx);
   wtx = newWtx(id);
   pthread_mutex_unlock(&ph->taMutex);
   if(!wtx){
      slog(ERROR,LOG_PLUGIN,"New transaction could not be created.");
      return 0; 
   }
   slog(DEBUG,LOG_PLUGIN,"New transaction %d created. WTX is at 0x%x",id,wtx);
   duk_push_number(ctx, id);
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
    { "newTransaction", js_newTransaction, 1},
    { "lock",      	js_lockTransaction, 1},
    { "commit",    	js_commitTransaction, 1},
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
   duk_put_global_string(ctx, "CTransaction");  /* -> stack: [ ] */

   wally_put_function("commit" ,WFUNC_THRD, c_commit, 0);

   slog(TRACE,LOG_PLUGIN,"Plugin initialized. PH is at 0x%x",ph);
   return PLUGIN_SCOPE;
}
