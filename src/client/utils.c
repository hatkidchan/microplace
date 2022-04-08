#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include "utils.h"
#include "../../raylib/src/raylib.h"

Color PALETTE_ANSI_VGA[16] = {
    {   0,   0,   0, 255 },
    { 170,   0,   0, 255 },
    {   0, 170,   0, 255 },
    { 170,  85,   0, 255 },
    {   0,   0, 170, 255 },
    { 170,   0, 170, 255 },
    {   0, 170, 170, 255 },
    { 170, 170, 170, 255 },
    {  85,  85,  85, 255 },
    { 255,  85,  85, 255 },
    {  85, 255,  85, 255 },
    { 255, 255,  85, 255 },
    {  85,  85, 255, 255 },
    { 255,  85, 255, 255 },
    {  85, 255, 255, 255 },
    { 255, 255, 255, 255 }
};

uint8_t COLOR256_BRIGHTNESS_LUT[6] = {
  0x00, 0x66, 0x88, 0xbb, 0xdd, 0xff
};

Color Color256(uint8_t i)
{
  Color c = WHITE;
  if (i < 16) return PALETTE_ANSI_VGA[i];
  else if (i >= 232)
  {
    c.r = c.g = c.b = (i - 232) * 255 / 24;
  }
  else
  {
    i -= 16;  c.b = COLOR256_BRIGHTNESS_LUT[i % 6];
    i /= 6;   c.g = COLOR256_BRIGHTNESS_LUT[i % 6];
    i /= 6;   c.r = COLOR256_BRIGHTNESS_LUT[i % 6];
  }
  return c;
}

#endif
