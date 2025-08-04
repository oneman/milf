#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <net/if.h>
#include <errno.h>
#include <string.h>

typedef float f32;
typedef double f64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;
typedef ssize_t isize;

u64 zero(void *buf, u64 sz) {
  u64 n = 0;
  char *bp = (char *)buf;
  for (n = 0; n < sz; n++) bp[n] = 0;
  return sz;
}

u64 mset(void *buf, const char b, u64 sz) {
  u64 n = 0;
  char *bp = (char *)buf;
  for (n = 0; n < sz; n++) bp[n] = b;
  return sz;
}
