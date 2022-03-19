#ifndef _CLIENT_NETWORKING_H_
#define _CLIENT_NETWORKING_H_

#include "cli_state.h"

void cnet_connect(state_t *state, const char *addr);
bool cnet_is_connected(state_t *state);

#endif
