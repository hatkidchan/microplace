#ifndef _CLIENT_PKHANDLERS_H_
#define _CLIENT_PKHANDLERS_H_

#include <stdlib.h>

void on_message(void *fnd, void *data, size_t size);
void on_error(void *fnd, char *msg);

#endif
