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

  \256\205\  1\  0\0\1\0\0\0\0\0\0\nradcompany\2us\0\0\34\0\1           len=31
  \256\205\201\200\0\1\0\0\0\1\0\0\nradcompany\2us\0\0\34\0\1\300
*/

void parse(int sz, char *dat) {
  watbuf(sz, dat);
  if (sz < 12) return;
  u16 id;
  u16 qs;
  u16 as;
  u16 ns;
  u16 aas;
  revcp(&id, dat, 2);
  revcp(&qs, &dat[4], 2);
  revcp(&as, &dat[6], 2);
  revcp(&ns, &dat[8], 2);
  revcp(&aas, &dat[10], 2);
  printf("id %u ", id);
  if ((dat[3] & 0b00000000) == 0b00000000) {
    printf("%u query ", qs);
  }
  if ((dat[3] & 0b10000000) == 0b10000000) {
    printf("%u query %u response ", qs, as);
  }
  if ((dat[4] & 0b00001111) != 0b00000000) {
    printf("error! ");
    return;
  }
  if (ns + aas) printf("%u ns %u aas\n", ns, aas);
  char label[64];
  int pos = 12;
  for (int i = 0; i < qs; i++) {
    for (;;) {
      if (pos > sz) break;
      int len = dat[pos];
      if (!len) { pos = pos + 1; break; }
      printf("len %d pos %d (len + pos = %d) sz %d", len, pos, len + pos, sz);
      if ((len > 63) || ((len + pos) > sz)) {
        printf("tooo len\n");
        return;
      }
      pos++;
      zero(label, 45);
      cp(label, &dat[pos], len);
      pos += len;
      printf("label %*s", len, label);
      u16 type;
      u16 class;
      if (pos + 4 > sz) { printf("nobytesfortypeclass\n"); return; }
      revcp(&type, &dat[pos], 2);
      pos = pos + 2;
      revcp(&class, &dat[pos], 2);
      pos = pos + 2;
      switch (type) {
        case 1:
          printf(" A ");
          break;
        case 2:
          printf(" NS ");
          break;
        case 5:
          printf(" CNAME ");
        break;
        case 6:
          printf(" SOA ");
          break;
        case 15:
          printf(" MX ");
          break;
        case 16:
          printf(" TXT ");
          break;
        case 28:
          printf(" AAAA ");
        break;
        case 225:
          printf(" * ");
          break;
        printf(" ERROR ");
        return;
      }
      printf("\n");
      if (class != 1) {
        printf("not internet class\n");
        return;
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc > 1) printf("%d arguments retarded.\n", argc - 1);
  //int epfd = epoll_create1(0);
  //printf("epfd %d\n", epfd);
  int ret = 0;
  //struct epoll_event ev;
  char addrstr[40];
  zero(addrstr, 40);
  struct sockaddr saddr;
  mset(&saddr, 0, sizeof(struct sockaddr));
  struct sockaddr_in *s = (struct sockaddr_in *)&saddr;
  struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)&saddr;
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
  //ev.events = EPOLLIN;
  //ev.data.fd = sd;
  //ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sd, &ev);
  //if (ret) { printf("epoll_ctlfail: %s\n", strerror(errno)); return 6; }
  printf("b1nd sd %d listens for querys on port %d\n", sd, 53);
  for (u64 i = 0;;i++) {
    printf("b1nd loop %zu\n", i);
    struct sockaddr from;
    socklen_t rsz = sizeof(from);
    char dat[4096];
    ret = recvfrom(sd, dat, 4096, 0, &from, &rsz);
    if (ret == 0) continue;
    if (ret == -1) { printf("recvfromfail: %s\n", strerror(errno)); return 11; }
    printf("%d bytes from ", ret);
    if (rsz == sizeof(struct sockaddr_in)) {
      s = (struct sockaddr_in *)&from;
      u8 ip[4];
      cp(ip, &s->sin_addr.s_addr, 4);
      u16 port = 0;
      revcp(&port, &s->sin_port, 2);
      //printf("%d %u - ", ntohs(s->sin_port), port);
      printf("%u.%u.%u.%u:%u\n", ip[0], ip[1], ip[2], ip[3], port);
      parse(ret, dat);
    }
    if (rsz == sizeof(struct sockaddr_in6)) {
      s6 = (struct sockaddr_in6 *)&from;
      u8 ip6[16];
      cp(ip6, &s6->sin6_addr.s6_addr, 16);
      u16 port = 0;
      revcp(&port, &s6->sin6_port, 2);
      //printf("%d %u - ", ntohs(s6->sin6_port), port);
      printf("ip6 %u:%u:%u:%u", ip6[0], ip6[1], ip6[2], ip6[3]);
      printf(":%u:%u:%u:%u", ip6[4], ip6[5], ip6[6], ip6[7]);
      printf(":%u:%u:%u:%u", ip6[8], ip6[9], ip6[10], ip6[11]);
      printf(":%u:%u:%u:%u %u", ip6[12], ip6[13], ip6[14], ip6[15], port);
    }
    printf("\n");
  }
  close(sd);
  return 2601;
}
