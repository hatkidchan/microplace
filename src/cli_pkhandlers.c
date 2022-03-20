#include "cli_pkhandlers.h"
#include "cli_pksender.h"
#include "cli_state.h"
#include "packets.h"
#include <assert.h>
#include <raylib.h>

const Color PALETTE_ANSI_VGA[16] = {
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

Color Color256(uint8_t i);

void on_message(void *fnd, void *data, size_t size)
{
  state_t *state = (state_t *)fnd;
  uint8_t *pk_in = (uint8_t *)data;
  uint8_t pk_type = *pk_in++;
  switch (pk_type)
  {
    case PK_S_INF:
      {
        printf("PK_S_INF\n");
        assert(size >= sizeof(pk_s_inf_t));
        pk_s_inf_t pkt = *(pk_s_inf_t *)pk_in;
        printf("pkt.cw: %d\n", state->world.cw = pkt.cw);
        printf("pkt.ch: %d\n", state->world.ch = pkt.ch);
        printf("pkt.nx: %d\n", state->world.nx = pkt.nx);
        printf("pkt.ny: %d\n", state->world.ny = pkt.ny);
        state->world.width = pkt.cw * pkt.nx;
        state->world.height = pkt.ch * pkt.ny;
        strncpy(state->world.name, pkt.name, 128);
        strncpy(state->world.description, pkt.description, 256);
        state->world.ready = true;
        state->canvas = LoadRenderTexture(state->world.width,\
            state->world.height);
        printf("done\n");
      }
      break;
    case PK_S_MSG:
      {
        printf("PK_S_MSG\n");
        pk_s_msg_t *pkt = (pk_s_msg_t *)pk_in;
        printf("pkt.r: %d\n", pkt->r);
        printf("pkt.g: %d\n", pkt->g);
        printf("pkt.b: %d\n", pkt->b);
        printf("pkt.data: %s\n", pkt->data);
        memmove(&state->chat[0], &state->chat[1], sizeof(chatmessage_t) * 15);
        state->chat[15].color.r = pkt->r;
        state->chat[15].color.g = pkt->g;
        state->chat[15].color.b = pkt->b;
        state->chat[15].color.a = 255;
        state->chat[15].phase = 1.0;
        strncpy(state->chat[15].text, pkt->data, 256);
        printf("pushed\n");
      }
      break;
    case PK_S_PIX:
      {
        printf("PK_S_PIX:\n");
        pk_s_pix_t *pkt = (pk_s_pix_t *)pk_in;
        printf("pkt.x: %d\n", pkt->x);
        printf("pkt.y: %d\n", pkt->y);
        printf("pkt.val: %02x\n", pkt->val);
        BeginTextureMode(state->canvas);
        Color rgb = Color256(pkt->val); rgb.a = 255;
        DrawRectangle(pkt->x, pkt->y, 1, 1, rgb);
        EndTextureMode();
      }
      break;
    case PK_S_CDT:
      {
        printf("PK_S_CDT: (%zd)\n", sizeof(pk_s_cdt_t) - sizeof(void*));
        pk_s_cdt_t pkt;
        pkt.type = *pk_in++;
        pkt.x = *pk_in++;
        pkt.y = *pk_in++;
        pkt.data = pk_in;
        state->world.has_chunk[pkt.x + pkt.y * 256] = true;
        int gox = pkt.x * state->world.cw,
            goy = pkt.y * state->world.ch;
        BeginTextureMode(state->canvas);
        for (int oy = 0; oy < state->world.ch; oy++)
        {
          for (int ox = 0; ox < state->world.cw; ox++)
          {
            uint8_t c = ((uint8_t*)pkt.data)[ox + oy * state->world.cw];
            Color rgb = Color256(c); rgb.a = 255;
            DrawRectangle(gox + ox, goy + oy, 1, 1, rgb);
          }
        }
        EndTextureMode();
        for (int y = 0; y < state->world.ny; y++)
        {
          for (int x = 0; x < state->world.nx; x++)
          {
            if (!state->world.has_chunk[x + y * 256])
            {
              pk_c_crq_t pkt = { .cx = x, .cy = y };
              send_pk_c_crq(state->conn, pkt);
              return;
            }
          }
        }
      }
      break;
  }
}

void on_error(void *fnd, char *msg)
{
  (void)fnd; (void)msg;
}

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
    i -= 16; c.b = (i % 6) * 42;
    i /= 6; c.g = (i % 6) * 42;
    i /= 6; c.r = (i % 6) * 42;
  }
  return c;
}
