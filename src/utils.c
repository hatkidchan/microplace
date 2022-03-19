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

#ifndef PLATFORM_WEB
const char *mg_ev_str(int ev)
{
  static const char _texts[32][16] = {
    "MG_EV_ERROR",
    "MG_EV_OPEN",
    "MG_EV_POLL",
    "MG_EV_RESOLVE",
    "MG_EV_CONNECT",
    "MG_EV_ACCEPT",
    "MG_EV_READ",
    "MG_EV_WRITE",
    "MG_EV_CLOSE",
    "MG_EV_HTTP_MSG",
    "MG_EV_HTTP_CHUNK",
    "MG_EV_WS_OPEN",
    "MG_EV_WS_MSG",
    "MG_EV_WS_CTL",
    "MG_EV_MQTT_CMD",
    "MG_EV_MQTT_MSG",
    "MG_EV_MQTT_OPEN",
    "MG_EV_SNTP_TIME",
    "MG_EV_USER"
  };
  switch (ev)
  {
    case MG_EV_ERROR:       return _texts[0]; break;
    case MG_EV_OPEN:        return _texts[1]; break;
    case MG_EV_POLL:        return _texts[2]; break;
    case MG_EV_RESOLVE:     return _texts[3]; break;
    case MG_EV_CONNECT:     return _texts[4]; break;
    case MG_EV_ACCEPT:      return _texts[5]; break;
    case MG_EV_READ:        return _texts[6]; break;
    case MG_EV_WRITE:       return _texts[7]; break;
    case MG_EV_CLOSE:       return _texts[8]; break;
    case MG_EV_HTTP_MSG:    return _texts[9]; break;
    case MG_EV_HTTP_CHUNK:  return _texts[10]; break;
    case MG_EV_WS_OPEN:     return _texts[11]; break;
    case MG_EV_WS_MSG:      return _texts[12]; break;
    case MG_EV_WS_CTL:      return _texts[13]; break;
    case MG_EV_MQTT_CMD:    return _texts[14]; break;
    case MG_EV_MQTT_MSG:    return _texts[15]; break;
    case MG_EV_MQTT_OPEN:   return _texts[16]; break;
    case MG_EV_SNTP_TIME:   return _texts[17]; break;
    case MG_EV_USER:        return _texts[18]; break;
  }
  return "???";
}
#endif

