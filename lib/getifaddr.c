#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_dl.h>
#include <ifaddrs.h>

int macaddr(char *ifname, char *macaddrstr) {
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

