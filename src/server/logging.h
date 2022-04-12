#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdio.h>

typedef enum {
  LOG_FATAL = 0,
  LOG_ERROR = 1,
  LOG_WARN = 2,
  LOG_INFO = 3,
  LOG_DEBUG = 4,
  LOG_TRACE = 5,
  LOG_ENDL = 6,
} loglevel_t;

#define log_TRACE(...) \
  log_main(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_DEBUG(...) \
  log_main(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_INFO(...) \
  log_main(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_WARN(...) \
  log_main(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_ERROR(...) \
  log_main(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_FATAL(...) \
  log_main(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#define loghex_TRACE(data, len) \
  log_hexdump(LOG_TRACE, __FILE__, __LINE__, data, len)
#define loghex_DEBUG(data, len) \
  log_hexdump(LOG_DEBUG, __FILE__, __LINE__, data, len)
#define loghex_INFO(data, len) \
  log_hexdump(LOG_INFO, __FILE__, __LINE__, data, len)
#define loghex_WARN(data, len) \
  log_hexdump(LOG_WARN, __FILE__, __LINE__, data, len)
#define loghex_ERROR(data, len) \
  log_hexdump(LOG_ERROR, __FILE__, __LINE__, data, len)
#define loghex_FATAL(data, len) \
  log_hexdump(LOG_FATAL, __FILE__, __LINE__, data, len)

extern loglevel_t __log_loglevel;
extern FILE *__log_fp;
void log_set_level(loglevel_t level);
void log_set_file(FILE *fp);
void log_main(loglevel_t lvl, const char *file, int line, const char *fmt, ...);
void log_hexdump(loglevel_t lvl, const char *f, int n, void *data, size_t len);
const char *log_levelstring(loglevel_t lvl);


#endif
