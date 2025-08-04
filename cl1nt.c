#include "stdio.h"

int main(int argc, char **argv) {
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
    if (rp->ai_family == AF_INET) {
      struct sockaddr_in *a = (struct sockaddr_in *)rp->ai_addr;
      printf("%d %u\n", ntohs(a->sin_port), ntohl(a->sin_addr.s_addr));
    }
    if (rp->ai_family == AF_INET6) {
      printf("ipv6 ok\n");
    }
  }
  return 1;   
}
