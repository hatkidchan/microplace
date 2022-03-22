#include "srv_state.h"
#include "srv_world.h"
#include "srv_handlers.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint8_t worldgen(uint16_t x, uint16_t y)
{
//  return (x ^ y ^ (x >> (y & 3)) ^ (y << (x & 3))) & 0xff;
  return (x + y) & 0xff;
}

int main(void)
{
  server_t server;
  if (!(server.world = world_open("world.data")))
  {
    perror("shit happened"); return 1;
  }
  
  if (!server.world->ready)
  {
    worldinfo_t opts = {
      .chunk_width = 40, .chunk_height = 40,
      .chunks_x = 25, .chunks_y = 25
    };
    strncpy(opts.name, "test world", 128);
    strncpy(opts.description, "sample text sample text", 256);
    printf("Generating new world...\n");
    if (!world_init(server.world, opts, worldgen))
    {
      perror("initialization failed"); return 2;
    }
  }
  
  printf("Loaded world %s\n", server.world->info->name);
  printf("Data offset: %zd (addr=%p)\n",
      server.world->data - (uint8_t *)server.world->_data,
      server.world->data);
  printf("World size: %dx%d (%dx%d chunks %dx%d each)\n",
      server.world->width, server.world->height,
      server.world->info->chunks_x,
      server.world->info->chunks_y,
      server.world->info->chunk_width,
      server.world->info->chunk_height);

  mg_mgr_init(&server.manager);
  mg_http_listen(&server.manager, "ws://0.0.0.0:8092/ws",
      handle_mongoose, &server);
  while (!server.should_stop)
    mg_mgr_poll(&server.manager, 1000);
  
  mg_mgr_free(&server.manager);

  world_close(server.world);
  return 0;
}
