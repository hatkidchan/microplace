#include "server/state.h"
#include "server/world.h"
#include "server/handlers.h"
#include "server/logging.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

uint8_t worldgen(uint16_t x, uint16_t y)
{
//  return (x ^ y ^ (x >> (y & 3)) ^ (y << (x & 3))) & 0xff;
  return (x + y) & 0xff;
}

int main(int argc, char **argv)
{
  log_set_file(stdout);
  char *world_path = "world.data";
  int c;
  bool regenerate = false;
  int port = 8092;
  while ((c = getopt(argc, argv, "gf:p:hl:")) != -1)
  {
    switch (c)
    {
      case 'g':
        regenerate = true;
        break;
      case 'f':
        world_path = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      case 'l':
        log_set_level(atoi(optarg));
        break;
    }
  }


  server_t server = { 0 };
  if (!(server.world = world_open(world_path)))
  {
    log_FATAL("Failed to open world: %s", strerror(errno));
    return 1;
  }

  if (!server.world->ready || regenerate)
  {
    worldinfo_t opts = {
      .chunk_width = 16, .chunk_height = 16,
      .chunks_x = 2, .chunks_y = 2
    };
    strncpy(opts.name, "test world", 128);
    strncpy(opts.description, "sample text sample text", 256);
    log_INFO("Generating new world");
    if (!world_init(server.world, opts, worldgen))
    {
      log_FATAL("World initialization failed: %s", strerror(errno));
      return 2;
    }
  }

  log_INFO("Loaded world: %s", server.world->info->name);
  log_DEBUG("Data offset: %zd (addr=%p)",
      server.world->data - (uint8_t *)server.world->_data,
      server.world->data);
  log_DEBUG("World size: %dx%d (%dx%d chunks %dx%d each)",
      server.world->width, server.world->height,
      server.world->info->chunks_x,
      server.world->info->chunks_y,
      server.world->info->chunk_width,
      server.world->info->chunk_height);

  char addr[128];
  snprintf(addr, 128, "ws://0.0.0.0:%d/ws", port);
  log_INFO("Listening on %s...", addr);
  mg_mgr_init(&server.manager);
  mg_http_listen(&server.manager, addr, handle_mongoose, &server);
  while (!server.should_stop)
    mg_mgr_poll(&server.manager, 1000);

  log_INFO("Shutting down...");
  mg_mgr_free(&server.manager);

  world_close(server.world);
  return 0;
}
