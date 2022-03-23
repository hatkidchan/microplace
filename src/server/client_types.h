#ifndef _S_CLIENT_TYPES_H_
#define _S_CLIENT_TYPES_H_

#include "../../mongoose/mongoose.h"
#include "world.h"
#include "state.h"

typedef struct client_s {
  uint64_t id;
  char username[128];
  char address[256];
  world_t *world;
  server_t *server;
  struct mg_connection *mgconn;
  struct client_s *next;
  bool can_write : 1;
  uint64_t last_placed_pixel_ts;
} client_t;

#endif
