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
  for (int p = 25; p <= 465; p += 440) {
    mset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (p == 25) ret = getaddrinfo(NULL, "25", &hints, &result);
    if (p == 465) ret = getaddrinfo(NULL, "465", &hints, &result);
    if (ret) { printf("gaif: %s\n", gai_strerror(ret)); return 2; }
    for (rp = result; rp != NULL; rp = rp->ai_next) {
      sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sd == -1) { printf("socketfail: %s\n", strerror(errno)); return 3; }
      ret = bind(sd, rp->ai_addr, rp->ai_addrlen);
      if (ret) { printf("bindfail: %s\n", strerror(errno)); return 4; }
      ret = listen(sd, 26);
      if (ret) { printf("listenfail: %s\n", strerror(errno)); return 5; }
      ev.events = EPOLLIN;
      ev.data.fd = sd;
      ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sd, &ev);
      if (ret) { printf("epoll_ctlfail: %s\n", strerror(errno)); return 6; }
      printf("milf sd %d listens for email on port %d\n", sd, p);
      break;
    }
    freeaddrinfo(result);
  }
  char buf[4096];
  for (;;) {
    ret = epoll_wait(epfd, &ev, 1, 0);
    if (ret) { printf("epoll_wait %d\n", ret); }
    if ((ev.events == EPOLLIN) && (ev.data.fd < 6)) {
      struct sockaddr sin;
      socklen_t slen = sizeof(sin);
      sd = accept(ev.data.fd, &sin, &slen);
      if (sd == -1) { printf("acceptfail: %s\n", strerror(errno)); }
      printf("new customer: %d\n", sd);
      write(sd, "220 OK rad \n\r", 13);
    }
    if ((ev.events == EPOLLIN) && (ev.data.fd > 5)) {
      for (int r = 4096; r == 4096;) {
        r = read(ev.data.fd, buf, r);
        if (r < 1) { printf("readfail: %d\n", r); }
        int w = write(1, buf, r);
        if (w != r) { printf("writefail: %d != %d\n", w, r); }
        write(sd, "250 OK rad \n\r", 13);
      }
    }
  }
  return 1;
}
