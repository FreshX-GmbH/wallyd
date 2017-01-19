#include "plugins.h"
#include "util.h"
#include <sys/types.h>
#include <netdb.h>
#ifdef DARWIN
#include <ifaddrs.h> 
#include <net/if_dl.h>
#endif
#ifdef LINUX
#include <sys/ioctl.h>
#include <net/if.h>
#endif

// TODO : make on version work on both OS

#ifdef DARWIN
int macaddr(const char *ifname, char *macaddrstr) {
    struct ifaddrs *ifap, *ifaptr;
    unsigned char *ptr;
    int ret = 0;
    if (getifaddrs(&ifap) == 0) {
        for(ifaptr = ifap; ifaptr != NULL; ifaptr = (ifaptr)->ifa_next) {
#ifdef DARWIN
            if (!strcmp((ifaptr)->ifa_name, ifname) && (((ifaptr)->ifa_addr)->sa_family == AF_LINK)) {
                ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)(ifaptr)->ifa_addr);
                sprintf(macaddrstr, "%02x:%02x:%02x:%02x:%02x:%02x",
                                    *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
                ret++;
                break;
            }
#endif
#ifdef LINUX
            if (!strcmp((ifaptr)->ifa_name, ifname) && (((ifaptr)->ifa_addr)->sa_family == AF_PACKET)) {
            	struct sockaddr_ll *s = (struct sockaddr_ll*)iface->ifa_addr;
				ptr = s->ssl_addr;
                sprintf(macaddrstr, "%02x:%02x:%02x:%02x:%02x:%02x",
                                    *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
                ret++;
                break;
            }
#endif
        }
        freeifaddrs(ifap);
        //return ifaptr != NULL;
        return ret;
    } else {
        return 0;
    }
}

#endif

#ifdef LINUX
int macaddr(const char *ifname, char *macp)
{
  char buf[8192] = {0};
  struct ifconf ifc = {0};
  struct ifreq *ifr = NULL;
  int sck = 0;
  int nInterfaces = 0;
  int i = 0;
  char ip[INET6_ADDRSTRLEN] = {0};
  struct ifreq *item;
  struct sockaddr *addr;

  slog(INFO,LOG_JS,"Get Mac for IF : %s",ifname);

  /* Get a socket handle. */
  sck = socket(PF_INET, SOCK_DGRAM, 0);
  if(sck < 0)
  {
    slog(ERROR,LOG_UTIL,"Error in socket.");
    return 0;
  }

  /* Query available interfaces. */
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if(ioctl(sck, SIOCGIFCONF, &ifc) < 0)
  {
    slog(ERROR,LOG_UTIL,"ioctl(SIOCGIFCONF)");
    return 0;
  }

  /* Iterate through the list of interfaces. */
  ifr = ifc.ifc_req;
  nInterfaces = ifc.ifc_len / sizeof(struct ifreq);

  for(i = 0; i < nInterfaces; i++)
  {
    item = &ifr[i];
    if(strcmp(ifname,item->ifr_name) !=0) continue;

//    addr = &(item->ifr_addr);
//
//    /* Get the IP address*/
//    if(ioctl(sck, SIOCGIFADDR, item) < 0)
//    {
//      perror("ioctl(OSIOCGIFADDR)");
//    }
//
//    if (inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), ip, sizeof ip) == NULL)
//        {
//           perror("inet_ntop");
//           continue;
//        }

    sprintf(macp, "%02x:%02x:%02x:%02x:%02x:%02x",
    (unsigned char)item->ifr_hwaddr.sa_data[0],
    (unsigned char)item->ifr_hwaddr.sa_data[1],
    (unsigned char)item->ifr_hwaddr.sa_data[2],
    (unsigned char)item->ifr_hwaddr.sa_data[3],
    (unsigned char)item->ifr_hwaddr.sa_data[4],
    (unsigned char)item->ifr_hwaddr.sa_data[5]);
    return 1;
  }
  slog(WARN,LOG_JS,"Mac of %s not found.",ifname);
  return 0;
}
#endif
