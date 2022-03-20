#ifndef _S_FILESYSTEM_H_
#define _S_FILESYSTEM_H_

#include <stdlib.h>

struct packed_file {
  const char *name;
  const unsigned char *data;
  size_t size;
  time_t mtime;
};

const char *mg_unlist(size_t no);
const char *mg_unpack(const char *name, size_t *size, time_t *mtime);

#endif
