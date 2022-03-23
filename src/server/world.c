#include "world.h"
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdio.h>

world_t *world_open(const char *fpath)
{
  int fd = open(fpath, O_RDWR | O_CREAT, 0664);
  if (fd < 0)
  {
    return NULL;
  }

  world_t *world = calloc(1, sizeof(world_t));
  world->_fd = fd;
  
  worldinfo_t info;
  ssize_t n_read = read(fd, &info, sizeof(worldinfo_t));
  if (n_read < 0)
  {
    int errcode = errno;
    free(world);
    close(fd);
    errno = errcode;
    return NULL;
  }
  else if (n_read == sizeof(worldinfo_t))
  {
    world->ready = true;
    world->width = info.chunk_width * info.chunks_x;
    world->height = info.chunk_height * info.chunks_y;
    size_t length = sizeof(worldinfo_t) + world->width * world->height;
    world->_data = mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (world->_data == MAP_FAILED)
    {
      int errcode = errno;
      close(fd);
      free(world);
      errno = errcode;
      return NULL;
    }
    world->info = (worldinfo_t *)world->_data;
    world->data = ((uint8_t *)world->_data) + sizeof(worldinfo_t);
    return world;
  }
  world->ready = false;
  return world;
}

bool world_init(world_t *world, worldinfo_t opts, worldgen_fn_t wgen)
{
  int fd = world->_fd;
  lseek(fd, 0, SEEK_SET);
  write(fd, &opts, sizeof(worldinfo_t));
  uint8_t stripe[65536];
  world->width = opts.chunk_width * opts.chunks_x;
  world->height = opts.chunk_height * opts.chunks_y;
  for (int y = 0; y < world->height; y++)
  {
    for (int x = 0; x < world->width; x++)
    {
      stripe[x] = wgen(x, y);
    }
    write(fd, stripe, world->width);
  }
  lseek(fd, 0, SEEK_SET);
  size_t length = sizeof(worldinfo_t) + world->width * world->height;
  world->_data = mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (world->_data == MAP_FAILED)
  {
    return false;
  }
  
  world->info = (worldinfo_t *)world->_data;
  world->data = ((uint8_t *)world->_data) + sizeof(worldinfo_t);
  
  world->ready = true;
  return true;
}

uint8_t *world_get(world_t *world, int x, int y)
{
  if (!world->ready) return NULL;
  if (x < 0 || y < 0 || x >= world->width || y >= world->height)
    return NULL;
  return &world->data[x + y * world->width];
}

bool world_set(world_t *world, int x, int y, uint8_t c)
{
  uint8_t *p = world_get(world, x, y);
  if (p == NULL) return false;
  *p = c;
  return true;
}

bool world_crop_chunk(world_t *world, int cx, int cy, uint8_t *dst)
{
  if (cx < 0 || cy < 0
      || cx >= world->info->chunks_x
      || cy >= world->info->chunks_y)
    return false;
  
  worldinfo_t info = *world->info;
  int w = info.chunk_width, h = info.chunk_height, x = cx * w, y = cy * h;
  
  for (int oy = 0; oy < h; oy++)
  {
    uint8_t *src = &world->data[(y + oy) * world->width + x];
    memcpy(&dst[oy * w], src, w);
  }


  return true;
}

void world_close(world_t *world)
{
  if (world->_data != MAP_FAILED && world->_data != NULL)
  {
    size_t length = world->width * world->height + sizeof(worldinfo_t);
    munmap(world->_data, length);
  }
  close(world->_fd);
  world->ready = false;
}
