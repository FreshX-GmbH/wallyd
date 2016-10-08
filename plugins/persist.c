#include "../lib/default.h"
#include "../lib/util.h"

// Depending on the archtiecture, this file creates the kernel argument lines
// as well as the SOC config file (i.e. config.txt on raspberry or uboot.env on uboot based systems)
bool persistConfig(hash_table *configMap)
{
     char *defaultConfig = KERNEL_DEFAULT_CONFIG;
     hash_table *persistMap = malloc(sizeof(hash_table));
     hash_table *deviceMap = malloc(sizeof(hash_table));
     ht_init(persistMap, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
     ht_init(deviceMap, HT_KEY_CONST | HT_VALUE_CONST, 0.05);

     if(!persistMap || !deviceMap){
         slog(LVL_QUIET,ERROR,"Could not allocate memory for persistMap");
         return false;
     }
     slog(DEBUG,DEBUG,"Persisting config to %s", DEFAULT_CMDLINE_TXT);

     if(ht_contains_simple(configMap,"updateServer")){
            ht_insert_simple(persistMap,"W_US",ht_get_simple(configMap,"updateServer"));
     }
     if(ht_contains_simple(configMap,"configServerType")){
            ht_insert_simple(persistMap,"W_CONNECT",ht_get_simple(configMap,"configServerType"));
     }
     // If the server type is manual, save the server
     if(ht_contains_simple(configMap,"configServer") && ht_compare(configMap,"configServerType","manual")){
            ht_insert_simple(persistMap,"W_SERVER",ht_get_simple(configMap,"configServer"));
     }
     if(ht_compare(configMap,"debug","true")){
            ht_insert_simple(persistMap,"W_DEBUG","1");
     }
     if(ht_compare(configMap,"lan.dhcp","true")){
        ht_insert_simple(persistMap,"W_IP","dhcp");
     } else {
        if( ht_contains_simple(configMap,"lan.ip") &&
            ht_contains_simple(configMap,"lan.cidr") &&
            ht_contains_simple(configMap,"lan.subnet") &&
            ht_contains_simple(configMap,"lan.gw") &&
            ht_contains_simple(configMap,"lan.dns"))
        {
            ht_insert_simple(persistMap,"W_IP",ht_get_simple(configMap,"lan.ip"));
            ht_insert_simple(persistMap,"W_NET",ht_get_simple(configMap,"lan.cidr"));
            ht_insert_simple(persistMap,"W_NETMASK",ht_get_simple(configMap,"lan.subnet"));
            ht_insert_simple(persistMap,"W_GW",ht_get_simple(configMap,"lan.gw"));
            ht_insert_simple(persistMap,"W_DNS",ht_get_simple(configMap,"lan.dns"));
        } else {
            slog(LVL_QUIET,ERROR,"Lan config incomplete. Using DHCP!");
            ht_insert_simple(persistMap,"W_IP","dhcp");
        }
     }
     // WLAN will only be configured if a driver is set
     // TODO : Support open WLANs (currently WPA/WPA2 only)
     if(ht_contains_simple(configMap,"wlan.driver") && !ht_compare(configMap,"wlan.driver", "none")){
          ht_insert_simple(persistMap,"W_WFI_DRIVER",ht_get_simple(configMap,"wlan.driver"));
          ht_insert_simple(persistMap,"W_WFI_PSK",ht_get_simple(configMap,"wlan.psk"));
          ht_insert_simple(persistMap,"W_WFI_SSID",ht_get_simple(configMap,"wlan.ssid"));
          if(ht_compare(configMap,"wlan.dhcp","true")){
             ht_insert_simple(persistMap,"W_WFI_IP","dhcp");
          } else {
             if( ht_contains_simple(configMap,"wlan.ip") &&
                 ht_contains_simple(configMap,"wlan.cidr") &&
                 ht_contains_simple(configMap,"wlan.subnet") &&
                 ht_contains_simple(configMap,"wlan.gw") &&
                 ht_contains_simple(configMap,"wlan.dns") &&
                 ht_contains_simple(configMap,"wlan.psk") &&
                 ht_contains_simple(configMap,"wlan.ssid"))
             {
                 ht_insert_simple(persistMap,"W_WFI_IP",ht_get_simple(configMap,"wlan.ip"));
                 ht_insert_simple(persistMap,"W_WFI_NET",ht_get_simple(configMap,"wlan.cidr"));
                 ht_insert_simple(persistMap,"W_WFI_NETMASK",ht_get_simple(configMap,"wlan.subnet"));
                 ht_insert_simple(persistMap,"W_WFI_GW",ht_get_simple(configMap,"wlan.gw"));
                 ht_insert_simple(persistMap,"W_WFI_DNS",ht_get_simple(configMap,"wlan.dns"));
             } else {
                 slog(LVL_QUIET,ERROR,"WLan config incomplete. Using DHCP!");
                 ht_insert_simple(persistMap,"W_WFI_IP","dhcp");
             }
          }
     }
     if(ht_contains_simple(configMap,"zoomFactor")){
            ht_insert_simple(persistMap,"W_ZOOM",ht_get_simple(configMap,"zoomFactor"));
     }

     // process raspberry parameters for config.txt
     // This is : overscan, hdmiGroup, displayRotate, hdmiMode
     //if(ht_compare(configMap,"disableOverscan","true")){
     //}

     FILE *fd = fopen(DEFAULT_CMDLINE_TXT,"w");
     // write out kernel parameter file (RPi : cmdline.txt or uBoot : uboot.env)
     // TODO : free pointer
     // TODO : check for cmdline length
     unsigned int num=0;
     void **keys = ht_keys(persistMap, &num);
     fprintf(fd,"%s ",defaultConfig);
     if(!fd) {
        slog(LVL_QUIET,ERROR,"Can not write temporary config to "DEFAULT_CMDLINE_TXT);
        return false;
     }
     for(int i = 0; i <= num; i++){
         fprintf(fd,"%s=%s ",keys[i],ht_get_simple(persistMap,keys[i]));
     }
     if(ht_contains_simple(configMap,"kernelCmdline")){
         fprintf(fd,"%s ",ht_get_simple(configMap,"kernelCmdline"));
     }
     fprintf(fd,"\n");
     fclose(fd);
     free(keys);

     FILE *output;
     output = popen (BIN_PERSIST, "r");
     if (!output) {
        slog(LVL_QUIET,ERROR, "Could not run "BIN_PERSIST);
        return false;
     }
     num = 0;
     while( 1 ){
        char *ret = fgetln(output, (size_t*)&num);
        if(!ret || num == 0 || errno) break;
        slog(DEBUG,DEBUG, "%s",ret);
     }

     if (pclose(output) != 0) {
        slog(LVL_QUIET,ERROR,"Could not finish run of "BIN_PERSIST);
        return false;
     }
     // write out device config file (RPi : config.txt or uBoot : uboot.env)
     return true;
}
