#include <curl/curl.h>
#include "../lib/default.h"
#include "../lib/util.h"
#include "../lib/json.h"
#include "../lib/plugins.h"
#include "../lib/connect.h"

char *commandURL=NULL;

hash_table *registerMap;
hash_table *commandMap;
hash_table *tempMap;

extern pluginHandler *ph;

bool registerClient(char *loc)
{
   char *registerURL=NULL;
   asprintf(&registerURL,"%s/register?uuid=%s&mac=%s&fw_version=%s&arch=%s&platform=%s&ip=%s",
         ph->location,
         // TODO : More generic!
         ph->uuid,
         ht_get_simple(ph->configFlagsMap,"W_MAC"),
         ht_get_simple(ph->configFlagsMap,"VERSION"),
         ht_get_simple(ph->configFlagsMap,"WALLY_ARCH"),
         ht_get_simple(ph->configFlagsMap,"WALLY_PLATFORM"),
         ht_get_simple(ph->configFlagsMap,"W_IPADDR"));
   slog(DEBUG,DEBUG,"RegURL : %s",registerURL);
 
   registerMap = malloc(sizeof(hash_table));
   ht_init(registerMap, HT_KEY_CONST | HT_VALUE_CONST, 0.05);

   ph->registered = parseJSON(registerMap,url_call(registerURL),NULL);
   if(registerURL){
      free(registerURL);
   }
   if(ph->registered){
         asprintf(&commandURL,"%s/command?uuid=%s", ph->location, ph->uuid);
         commandURL=replace(commandURL,"/register?","/command?");
   } else {
         commandURL=NULL;
   }
   slog(DEBUG,DEBUG,"CmdURL : %s",commandURL);

   return ph->registered;
}

