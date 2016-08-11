#include "ssdp.h"
#include "../src/nucly/duv.h"

#define PLUGIN_SCOPE "ssdp"

pluginHandler *ph;
int port = 1900;

// Parse the buffer line by line, find the ID and the URL
// ST: #SSDP_ST_MATCH
// LOCATION: <REGISTER_URL>
bool parseSSDPResponse(char *buffer)
{
    bool ret = false;
    char * line = strtok(strdup(buffer), "\n");
    char *loc=NULL;
    while(line) {
        if (strncmp(line, "ST: ", 4) == 0) { 
            if (strstr(line, SSDP_ST_MATCH) != NULL) { 
                slog(LVL_INFO,INFO,"Matching field %s",line); 
                ret = true;
            }
        }
        if (strncmp(line, "LOCATION: ", 10) == 0) { 
            slog(LVL_NOISY,FULLDEBUG,"Matching field %s",line);
            asprintf(&loc,"%s",replace(replace(replace(line,"LOCATION: ",""),"\n",""),"\r",""));
        }
        line  = strtok(NULL, "\n");
    }
    // if both test succeeded, assign location globally
    // TODO : Free global location ptr
    if(ret) {
        ph->location = loc;
        slog(LVL_NOISY,DEBUG,"Location saved.");
    // otherwise free the ptr
    } else if(loc){
        slog(LVL_NOISY,DEBUG,"Location ignored.");
        free(loc);
    }
    return ret;
}

void ssdpDiscovery(int *_sl){
    int sl = *_sl;
    int count=0;
    char *buf,*logStr;
    int size=0;
    int ret;

    char *ptBuf=malloc(sl);
    memset(ptBuf,'.',sl);
    ptBuf[sl]='\0';
    asprintf(&logStr,"Searching for a Wallaby Server..%s",ptBuf);
    sendWallyCommand("log",logStr);
    free(logStr);
    free(ptBuf);

    int sd = sendBroadcastPacket();

    // We loop here forever since we get an error from the Broadcast sending
    // This is an inacceptable state for all the following wallyTV functions
    while(!sd) {
        slog(LVL_NOISY,DEBUG,"Timeout delay : %d",sl);
        sleep(sl);
        sd = sendBroadcastPacket();
        if(sl < 10) sl++;
        // give up
        if(sl > 20) return;
    }

    buf=malloc(2048);

    ph->ssdpRunning = true;
   
    // Endless SSDP Discover loop (until we break)
    while(ph->ssdpRunning){
        size = recv(sd,buf,2048,MSG_DONTWAIT); 
        if(size>0){
            // we are done if the parse is successful
            if(parseSSDPResponse(buf)){
                //saveLocation(DEFAULT_LOCATION_FILE);
                // This is invoked (and done) by the client plugin
                break;
            }
        } else {
            count++;
            // After 10 tries we restart the sending process
            // Maybe we missed the server response??
            if(count > 10){
                count = 0;
                slog(LVL_INFO,INFO,"No response. Resending discovery packet.");
                if(sl < 10) { sl++; }
                ssdpDiscovery(&sl);
                break;
            }
            sleep(sl);
        }
    }
    ph->ssdpRunning = false;
    free(buf);
    close(sd);
    ph->location=replace(ph->location,"/register","");
    asprintf(&logStr,"Found a Wallaby Server at : %s",ph->location);
    sendWallyCommand("log",logStr);
    free(logStr);
}

//void callSsdpDiscovery(char *timeout){
//   int sl=0;
//   getNum(timeout,&sl);
//   ssdpDiscovery(&sl);
//}

int sendBroadcastPacket(void) 
{
    // Open a socket
    int sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sd<=0) {
        slog(LVL_QUIET,ERROR,"Error: Could not open socket");
        return false;
    }
    
    // Set socket options
    int broadcastEnable=1;
    int ret=setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    if (ret) {
        slog(LVL_QUIET,ERROR,"Error: Could not open set socket to broadcast mode");
        close(sd);
        return false;
    }
    
    // Configure the port and ip we want to send to
    struct sockaddr_in broadcastAddr; // Make an endpoint
    memset(&broadcastAddr, 0, sizeof broadcastAddr);
    broadcastAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "239.255.255.250", &broadcastAddr.sin_addr); // Set the broadcast IP address
    broadcastAddr.sin_port = htons(port); // Set port
    
    char *request = "M-SEARCH * HTTP/1.1\r\nHOST:239.255.255.250:1900\r\nMAN:\"ssdp:discover\"\r\nST:ssdp:all\r\nNT:freshx:wally\r\nMX:1\r\n\r\n";
    ret = sendto(sd, request, strlen(request), 0, (struct sockaddr*)&broadcastAddr, sizeof broadcastAddr);
    if (ret<0) {
        slog(LVL_QUIET,ERROR,"Error: Could not open send broadcast : %s",strerror(errno));
        close(sd);
        return false; 
    }

    return sd;
}
char *cleanupPlugin(void *p){
   slog(LVL_INFO,INFO,"Cleanup ssdp plugin");
   return NULL;
}

const function_list_entry c_SSDPMethods[] = {
   { PLUGIN_SCOPE"::discovery", WFUNC_SYNC, ssdpDiscovery },
   { NULL, 0, NULL, 0}
};

char *initPlugin(pluginHandler *_ph){
    ph=_ph;
    wally_put_function_list(c_SSDPMethods);
    slog(LVL_NOISY,FULLDEBUG,"Plugin ssdp initializing. PH is at 0x%x",ph);
    return PLUGIN_SCOPE;
}

duk_ret_t duv_udp_send(duk_context *ctx) {
  dschema_check(ctx, (const duv_schema_entry[]) {
    {"host", duk_is_string},
    {"port", duk_is_number},
    {"data", duk_is_string},
    {"callback", dschema_is_continuation},
    {0,0}
  });
  return 0;
}
