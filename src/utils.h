#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include <stdint.h>

#ifndef PLATFORM_WEB
// fuck windows.h
#ifdef _WIN32
#define _WINSOCK2API_
#define _WS2TCPIP_H_
#endif
#include "../mongoose/mongoose.h"
#endif

#ifndef strnlen
#define __U_CUSTOM_STRNLEN
size_t strnlen(const char *s, size_t maxlen);
#endif

uint64_t now_ms(void);
#ifndef PLATFORM_WEB
const char *mg_ev_str(int ev);
#endif

#endif
