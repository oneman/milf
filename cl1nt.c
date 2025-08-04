#include "stdio.h"

       #define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
       #include <arpa/inet.h>
       #include <sys/socket.h>
       #include <netdb.h>
       #include <ifaddrs.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <linux/if_link.h>

       void nfo(void)
       {
           struct ifaddrs *ifaddr;
           int family, s;
           char host[NI_MAXHOST];

           if (getifaddrs(&ifaddr) == -1) {
               perror("getifaddrs");
               exit(EXIT_FAILURE);
           }

           /* Walk through linked list, maintaining head pointer so we
              can free list later. */

           for (struct ifaddrs *ifa = ifaddr; ifa != NULL;
                    ifa = ifa->ifa_next) {
               if (ifa->ifa_addr == NULL)
                   continue;
           if ((ifa->ifa_name[0] == 'l') && (ifa->ifa_name[1] == 'o')) continue;


               family = ifa->ifa_addr->sa_family;

               /* Display interface name and family (including symbolic
                  form of the latter for the common families). */

               printf("%-8s %s (%d)\n",
                      ifa->ifa_name,
                      (family == AF_PACKET) ? "AF_PACKET" :
                      (family == AF_INET) ? "AF_INET" :
                      (family == AF_INET6) ? "AF_INET6" : "???",
                      family);

               /* For an AF_INET* interface address, display the address. */

               if (family == AF_INET || family == AF_INET6) {
                   s = getnameinfo(ifa->ifa_addr,
                           (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                 sizeof(struct sockaddr_in6),
                           host, NI_MAXHOST,
                           NULL, 0, NI_NUMERICHOST);
                   if (s != 0) {
                       printf("getnameinfo() failed: %s\n", gai_strerror(s));
                       exit(EXIT_FAILURE);
                   }

                   printf("\t\taddress: <%s>\n", host);

               } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
                   struct rtnl_link_stats *stats = ifa->ifa_data;

                   printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                          "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                          stats->tx_packets, stats->rx_packets,
                          stats->tx_bytes, stats->rx_bytes);
               }
           }

           freeifaddrs(ifaddr);
           //exit(EXIT_SUCCESS);
       }

int main(int argc, char **argv) {
  nfo();
  if (argc > 3) printf("Arguments Ignored\n");
  int ret = 0;
  struct addrinfo *result;
  struct addrinfo *rp;
  char *name = "radcompany.us";
  char *type = "80";
  if (argc > 1) name = argv[1];
  if (argc > 2) type = "25";
  ret = getaddrinfo(name, type, NULL, &result);
  if (ret) { printf("gaif: %s\n", gai_strerror(ret)); return 2; }
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    printf("%d|%d|%d|%d|%u|%s\n", rp->ai_flags, rp->ai_family, rp->ai_socktype,
      rp->ai_protocol, rp->ai_addrlen, rp->ai_canonname);
    if (rp->ai_family == AF_INET) {
      struct sockaddr_in *a = (struct sockaddr_in *)rp->ai_addr;
      u8 ip[4];
      cp(ip, &a->sin_addr.s_addr, 4);
      u16 port = 0;
      revcp(&port, &a->sin_port, 2);
      printf("%d %u - ", ntohs(a->sin_port), port);
      printf("ip %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
    }
    if (rp->ai_family == AF_INET6) {
      struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)rp->ai_addr;
      u8 ip6[16];
      cp(ip6, &a6->sin6_addr.s6_addr, 16);
      u16 port = 0;
      revcp(&port, &a6->sin6_port, 2);
      printf("%d %u - ", ntohs(a6->sin6_port), port);
      printf("ip6 %u:%u:%u:%u", ip6[0], ip6[1], ip6[2], ip6[3]);
      printf(":%u:%u:%u:%u", ip6[4], ip6[5], ip6[6], ip6[7]);
      printf(":%u:%u:%u:%u", ip6[8], ip6[9], ip6[10], ip6[11]);
      printf(":%u:%u:%u:%u\n", ip6[12], ip6[13], ip6[14], ip6[15]);
    }
  }
  return 1;   
}
