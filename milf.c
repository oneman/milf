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
  //for (int p = 25; p <= 465; p += 440) {
  for (int p = 25; p <= 25; p += 440) {
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
      const int one = 1;
      if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const void *)&one, 4)) {
        printf("setsockoptfail: %s\n", strerror(errno)); return 9;
      }
      if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (const void *)&one, 4)) {
        printf("setsockoptfail: %s\n", strerror(errno)); return 9;
      }
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
    ret = epoll_wait(epfd, &ev, 1, -1);
    if (ret == -1) {
      printf("epoll_waitfail: %s\n", strerror(errno));
      return 15;
    }
    if (ret == 0) {
      printf("epoll_wait %d\n", ret);
      continue;
    }
    if ((ev.events & EPOLLIN) && (ev.data.fd < 5)) {
      struct sockaddr sin;
      socklen_t slen = sizeof(sin);
      sd = accept(ev.data.fd, &sin, &slen);
      if (sd == -1) { printf("acceptfail: %s\n", strerror(errno)); }
      printf("new customer: %d\n", sd);
      write(sd, "220 OK rad \r\n", 13);
      ev.events = EPOLLIN;
      ev.data.fd = sd;
      ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sd, &ev);
      if (ret) { printf("epoll_ctlfail: %s\n", strerror(errno)); return 6; }
    }
    if ((ev.events & EPOLLIN) && (ev.data.fd > 4)) {
      for (int r = 4096; r == 4096;) {
        r = read(ev.data.fd, buf, r);
        if (r < 1) {
          if (r == -1) printf("readfail: %s\n", strerror(errno));
          printf("read: %d from %d we close\n", r, ev.data.fd);
          ret = epoll_ctl(epfd, EPOLL_CTL_DEL, ev.data.fd, NULL);
          if (ret) { printf("epoll_ctlfail: %s\n", strerror(errno)); return 9; }
          ret = close(ev.data.fd);
          if (ret) { printf("closefail: %s\n", strerror(errno)); return 13; }
          break;
        }
        watbuf(r, buf);
        int w = write(1, buf, r);
        if (w != r) { printf("writefail: %d != %d\n", w, r); }
        if (buf[0] == 'D') {
          write(ev.data.fd, "354 OK rad \r\n", 13);
        } else { 
          write(ev.data.fd, "250 OK rad \r\n", 13);
        }
      }
    }
  }
  return 1;
}
