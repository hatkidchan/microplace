#include "logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

loglevel_t __log_loglevel = LOG_INFO;
FILE *__log_fp = NULL;
const char *__log_levelstrings[LOG_ENDL] = {
  "CRT", "ERR", "WRN", "INF", "DBG", "TRC"
};


void log_set_level(loglevel_t level)
{
  __log_loglevel = level;
}

void log_set_file(FILE *fp)
{
  __log_fp = fp == NULL ? stdout : fp;
}

void log_main(loglevel_t lvl, const char *file, int line, const char *fmt, ...)
{
  if (__log_loglevel < lvl) return;

  char date_string[32];
  time_t timestamp = time(NULL);
  struct tm *timeinfo = localtime(&timestamp);
  strftime(date_string, 32, "%Y-%m-%d %H:%M:%S", timeinfo);

  char where[128];
  snprintf(where, 128, "%s:%d", file, line);
  fprintf(__log_fp, "%s %3s %-25s: ", date_string, log_levelstring(lvl), where);

  va_list args;
  va_start(args, fmt);
  vfprintf(__log_fp, fmt, args);
  va_end(args);

  fputc('\n', __log_fp);
  fflush(__log_fp);
}

void log_hexdump(loglevel_t lvl, const char *f, int n, void *data, size_t len)
{
  if (__log_loglevel < lvl) return;

  char now_str[32];
  time_t timestamp = time(NULL);
  struct tm *timeinfo = localtime(&timestamp);
  strftime(now_str, 32, "%Y-%m-%d %H:%M:%S", timeinfo);

  char where[128];
  snprintf(where, 128, "%s:%d", f, n);

  uint8_t *ptr = data;
  for (size_t i = 0; i < len;)
  {
    fprintf(__log_fp, "%s %3s %-25s: %08zx  ", now_str,
        log_levelstring(lvl), where, i);
    for (int x = 0; x < 16; x++)
    {
      if (i + x >= len) fprintf(__log_fp, "-- ");
      else fprintf(__log_fp, "%02x ", ptr[i + x]);
    }
    fprintf(__log_fp, "   |");
    for (int x = 0; x < 16; x++)
    {
      if (i + x >= len) break;
      else if (isprint(ptr[i + x])) fputc(ptr[i + x], __log_fp);
      else fputc('.', __log_fp);
    }
    fprintf(__log_fp, "|\n");
    i += 16;
  }
}

const char *log_levelstring(loglevel_t lvl)
{
  if (lvl >= LOG_TRACE) return __log_levelstrings[LOG_TRACE];
  return __log_levelstrings[lvl];
}
