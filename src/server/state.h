#ifndef _S_STATE_H_
#define _S_STATE_H_

#include "../../mongoose/mongoose.h"
#include "world.h"

typedef struct {
  world_t *world;
  struct mg_mgr manager;
  struct client_s *clients_head;
  uint32_t n_clients;
  bool should_stop;
} server_t;

#endif
