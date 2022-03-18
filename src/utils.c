#include "utils.h"
#include <sys/time.h>

#ifdef __U_CUSTOM_STRNLEN
// http://stackoverflow.com/questions/21536064/ddg#21536247
size_t strnlen(const char * s, size_t len) {
    size_t i = 0;
    for ( ; i < len && s[i] != '\0'; ++i);
    return i;
}
#endif

uint64_t now_ms()
{
  struct timeval now;
  gettimeofday(&now, 0);
  
  uint64_t ms = now.tv_sec * 1000 + now.tv_usec / 1000;
  return ms;
}

