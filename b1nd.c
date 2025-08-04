#include "stdio.h"

/*

0-1   - arb number / reply number
2-3   - 0x0100 query / response 0x8180              buf[2] = 1
4-5   - #?   > 0                                   buf[5] > 0
6-7   - #! 
8-9   - #NS
10-11 - #AD

if ((buf[2] == 1) && (buf[3] == 0)) // is a query

(buf[5] > 0) && (buf[5] < 26)) {

qsz = buf[12];
if (buf[12 + qsz] +/- 1) == 0)fa

#    ldh|\0  | 0x0001|0x0001
[sz|chars]|null|type|class


answer

 2byte           2byte  2byte   4byte         2byte
               0x0001 0x0001 0x00000258 
      pointer | type| class| ttl       |            datalen | data


sz = 1-63


a 1, ns 2, cname 5, soa 6, mx 15, txt 16, aaaa 28, * 255

labelfmt:
1 byte len 1-63


subformats:
 a:                4 bytes
 aaaa:             16 bytes
 ns                <domain-name>
 cname             <domain-name>
 soa
 mx:       0x0001 | <domain-name>
 txt: 
 
IPPROTO_TCP TCP_NODELAY


*/

int main(int argc, char **argv) {
  if (argc > 1) printf("%d arguments retarded.\n", argc - 1);
  int epfd = epoll_create1(0);
  printf("epfd %d\n", epfd);
  int ret = 0;
  struct epoll_event ev;
  char addrstr[40];
  zero(addrstr, 40);
  struct sockaddr saddr;
  mset(&saddr, 0, sizeof(struct sockaddr));
  struct sockaddr_in *s = (struct sockaddr_in *)&saddr;
  s->sin_family = AF_INET;
  s->sin_port = htons(53);
  s->sin_addr.s_addr = htonl(INADDR_ANY);
  int sd = socket(s->sin_family, SOCK_DGRAM, 0);
  if (sd == -1) { printf("socketfail: %s\n", strerror(errno)); return 3; }
  const int one = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const void *)&one, 4)) {
    printf("setsockoptfail: %s\n", strerror(errno)); return 9;
  }
  ret = bind(sd, &saddr, sizeof(struct sockaddr_in));
  if (ret) { printf("bindfail: %s\n", strerror(errno)); return 4; }
  ev.events = EPOLLIN;
  ev.data.fd = sd;
  ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sd, &ev);
  if (ret) { printf("epoll_ctlfail: %s\n", strerror(errno)); return 6; }
  printf("b1nd sd %d listens for querys on port %d\n", sd, 53);
  close(sd);
  return 2601;
}
