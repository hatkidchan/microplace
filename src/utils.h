#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include <stdint.h>

#ifndef strnlen
#define __U_CUSTOM_STRNLEN
size_t strnlen(const char *s, size_t maxlen);
#endif

uint64_t now_ms(void);

#endif
