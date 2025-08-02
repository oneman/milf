#include "stdio.h"

int main(int argc, char **argv) {
  if (argc > 1) printf("Arguments Ignored\n");
  int epfd = epoll_create1(0);
  int ret = 0;
  struct epoll_event ev;
  int sd = 0;
  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *rp;
  mset(&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;     /* All interfaces */
  ret = getaddrinfo(NULL, "25", &hints, &result);
  if (ret) { printf("we failed %s\n", gai_strerror(ret)); return 2; }
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sd == -1) { printf("fail %s\n", strerror(errno)); return 3; }
    ret = bind(sd, rp->ai_addr, rp->ai_addrlen);
    if (ret) { printf("we2failed %s\n", strerror(errno)); return 4; }
    ret = listen(sd, 26);
    if (ret) { printf("we3failed %s\n", strerror(errno)); return 5; }    
    ev.events = EPOLLIN;
    ev.data.fd = sd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sd, &ev);
    if (ret) { printf("we4failed %s\n", strerror(errno)); return 6; }
    printf("yay we did well with %d\n", sd);
    break;
  }
  freeaddrinfo(result);
  for (;;) {
    ret = epoll_wait(epfd, &ev, 1, 0);
    if (ret) { printf("epoll_wait %d\n", ret); }
  }
  return 1;
}
