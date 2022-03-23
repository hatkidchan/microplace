#ifndef _S_WORLD_H_
#define _S_WORLD_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint8_t chunks_x, chunks_y;         // +2   => 2
  uint8_t chunk_width, chunk_height;  // +2   => 4
  uint8_t _pad[4];                    // +4   => 8
  char name[128];                     // +128 => 136
  char description[256];              // +256 => 392
  uint64_t n_changes;                 // +8   => 400
  uint64_t n_messages;                // +8   => 408
  uint64_t n_connections;             // +8   => 416
  uint8_t _pad2[96];                  // +96  => 512
} worldinfo_t;

typedef struct world_s {
  void *_data;
  worldinfo_t *info;
  uint8_t *data;
  int _fd;
  int width, height;
  bool ready;
} world_t;

typedef uint8_t (*worldgen_fn_t)(uint16_t x, uint16_t y);

world_t *world_open(const char *fpath);
bool world_init(world_t *world, worldinfo_t opts, worldgen_fn_t wgen);
uint8_t *world_get(world_t *world, int x, int y);
bool world_set(world_t *world, int x, int y, uint8_t c);
bool world_crop_chunk(world_t *world, int cx, int cy, uint8_t *dst);
void world_close(world_t *world);

#endif
