#include "state.h"
#include "networking.h"
#include "../packets.h"
#include "pksender.h"
#include "../../raylib/src/raylib.h"
#include "../../raygui/src/raygui.h"
#include <math.h>

void __handle_pan_zoom(state_t *state);
void __render_flickers(state_t *state);

void handle_state_boot(state_t *state)
{
  state->state = CLST_LOGIN_SCREEN;
  memset(state->s_login_points_clr, 0, sizeof(state->s_login_points_clr));
  memset(state->s_login_points_pos, 0, sizeof(state->s_login_points_pos));
  for (int i = 0; i < 256; i++)
  {
    state->s_login_points_pos[i].x = GetRandomValue(0, state->width / 8);
    state->s_login_points_pos[i].y = GetRandomValue(0, state->height / 8);
    state->s_login_points_clr[i].r = GetRandomValue(128, 255);
    state->s_login_points_clr[i].g = GetRandomValue(128, 255);
    state->s_login_points_clr[i].b = GetRandomValue(128, 255);
    state->s_login_points_clr[i].a = GetRandomValue(128, 255);
  }
}

void handle_state_login_screen(state_t *state)
{
  BeginDrawing();
  {
    if (state->canvas.id != 0)
    {
      UnloadRenderTexture(state->canvas);
      state->canvas.id = 0;
    }
    ClearBackground(BLACK);
    
    __render_flickers(state);

    const char *text = "micro/place";
    int tw = MeasureText(text, 32);
    int y = (state->height - 200) / 2;
    DrawText(text, (state->width - tw) / 2, y, 32, GRAY);
    y += 48;
    Rectangle rec = {
      .x = (state->width - 400) / 2., .y = y,
      .width = 400, .height = 40
    };
    if (GuiTextBox(rec, state->server_address, 128, state->server_address_ed))
      state->server_address_ed ^= 1;
    
    rec.y += rec.height + 10;

    if (GuiButton(rec, "Connect"))
    {
      state->state = CLST_CONNECTING;
      state->timer_started_frame = state->frame;
      state->world.ready = false;
      state->sock_dropped = false;
      state->sock_connected = false;
      cnet_connect(state, state->server_address);
    }
    rec.y += rec.height + 10;
    DrawFPS(8, 8);
    DrawText(TextFormat("n=%d", state->frame), 98, 8, 16, BLUE);
  }
  EndDrawing();
  state->frame++;
}

void handle_state_connecting(state_t *state)
{
  BeginDrawing();
  {
    ClearBackground(BLACK);
    __render_flickers(state);
    const char *text = "Connecting to the server";
    int tw = MeasureText(text, 32) + 66;
    int x = (state->width - tw) / 2,
        y = (state->height - 32) / 2;
    DrawText(text, x, y, 32, GRAY);
    x += tw + 16;
    Vector2 center = { .x = x + 32, .y = y + 16 };
    float phase = (float)-state->frame * 2.0;
    DrawRing(center, 30, 32, phase, phase + 60.0, 50, GRAY);
    if (state->frame >= state->timer_started_frame + 480)
      state->state = CLST_CONNECTION_FAILED;
    else if (cnet_is_connected(state))
      state->state = CLST_EXCHANGING;
    else if (state->sock_dropped)
      state->state = CLST_CONNECTION_FAILED;
    DrawFPS(8, 8);
    DrawText(TextFormat("n=%d", state->frame), 98, 8, 16, BLUE);
  }
  EndDrawing();
  state->frame++;
}

void handle_state_exchanging(state_t *state)
{
  BeginDrawing();
  {
    ClearBackground(BLACK);
    __render_flickers(state);
    const char *text = "Receiving data...";
    int tw = MeasureText(text, 32) + 66;
    int x = (state->width - tw) / 2,
        y = (state->height - 32) / 2;
    DrawText(text, x, y, 32, GRAY);
    if (state->world.ready)
    {
      pk_c_crq_t pkt = { .cx = 0, .cy = 0 };
      send_pk_c_crq(state, pkt);
      state->state = CLST_MAINLOOP;
    }
    DrawFPS(8, 8);
    DrawText(TextFormat("n=%d", state->frame), 98, 8, 16, BLUE);
  }
  EndDrawing();
  state->frame++;
}

void handle_state_mainloop(state_t *state)
{
  BeginDrawing();
  {
    ClearBackground(BLACK);
    
    __handle_pan_zoom(state);
    
    
    BeginMode2D(state->camera);
    {
      Rectangle source = {
        .x = 0, .y = 0,
        .width = state->canvas.texture.width,
        .height = -state->canvas.texture.height,
      };
      Vector2 pos = {
        .x = 0, .y = 0
      };
      DrawRectangle(0, 0, state->world.width, state->world.height, BLUE);
      DrawRectangle(0, 0, 8, 8, RED);
      DrawTextureRec(state->canvas.texture, source, pos, WHITE);
    }
    EndMode2D();
    
    Vector2 mouse = GetMousePosition();
    Vector2 worldpos = GetScreenToWorld2D(mouse, state->camera);
    worldpos.x = ceilf(worldpos.x - 1.0);
    worldpos.y = ceilf(worldpos.y - 1.0);
    
    const char *txt = TextFormat("%d:%d", (int)worldpos.x, (int)worldpos.y);
    for (int oy = -1; oy <= 1; oy++)
    {
      for (int ox = -1; ox <= 1; ox++)
      {
        DrawText(txt, mouse.x + ox, mouse.y + oy, 14, BLACK);
      }
    }
    DrawText(txt, mouse.x, mouse.y, 14, WHITE);

    for (int i = 0; i < 16; i++)
    {
      int y = state->height - (16 - i) * 20;
      chatmessage_t *msg = &state->chat[i];
      if (msg->phase <= 0.01) continue;
      
      float alpha = msg->phase >= 0.5 ? 1.0 : msg->phase / 0.5;

      for (int oy = -1; oy <= 1; oy++)
      {
        for (int ox = -1; ox <= 1; ox++)
        {
          DrawText(msg->text, 8 + ox, y + oy, 20, Fade(BLACK, alpha));
        }
      }
      DrawText(msg->text, 8, y, 20, Fade(msg->color, alpha));
      msg->phase *= 0.999;
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
      pk_c_set_t pkt = {
        .x = worldpos.x,
        .y = worldpos.y,
        .val = state->selected_pix
      };
      send_pk_c_set(state, pkt);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
    {
      pk_c_set_t pkt = {
        .x = worldpos.x,
        .y = worldpos.y,
        .val = state->selected_pix
      };
      send_pk_c_set(state, pkt);
    }
    
    DrawFPS(8, 8);
    DrawText(TextFormat("n=%d", state->frame), 98, 8, 16, BLUE);
  }
  EndDrawing();
  state->frame++;
}

void handle_state_connection_failed(state_t *state)
{
  BeginDrawing();
  {
    ClearBackground(BLACK);
    const char *text = "Connection failed";
    int tw = MeasureText(text, 32) + 66;
    int x = (state->width - tw) / 2,
        y = (state->height - 32) / 2;
    DrawText(text, x, y, 32, RED); y += 48;
    Rectangle rec = {
      .x = (state->width - 400) / 2., .y = y,
      .width = 400, .height = 40
    };
    if (GuiButton(rec, "Try again"))
    {
      cnet_connect(state, state->server_address);
      state->timer_started_frame = state->frame;
      state->state = CLST_CONNECTING;
      state->world.ready = false;
    }
    rec.y += rec.height + 20;
    if (GuiButton(rec, "Back to login screen"))
    {
      state->state = CLST_LOGIN_SCREEN;
      state->world.ready = false;
    }
  }
  EndDrawing();
  state->frame++;
}

void handle_state_reconnect_begin(state_t *state)
{
  
}

void handle_state_reconnect_wait(state_t *state)
{
  
}

void handle_state_reconnect_failed(state_t *state)
{
  
}

void handle_state_kicked(state_t *state)
{
  
}

void __handle_pan_zoom(state_t *state)
{
  Vector2 mouse = GetMousePosition();
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    state->c_pan_start = mouse;
    state->c_pan_src_target = state->camera.target;
  }
  
  if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
  {
    Vector2 delta = {
      .x = (state->c_pan_start.x - mouse.x) / state->camera.zoom,
      .y = (state->c_pan_start.y - mouse.y) / state->camera.zoom,
    };
    state->camera.target.x = state->c_pan_src_target.x + delta.x;
    state->camera.target.y = state->c_pan_src_target.y + delta.y;
  }
  
  float scroll = GetMouseWheelMove();
  if (scroll != 0)
  {
    Vector2 mouse_bz = GetScreenToWorld2D(mouse, state->camera);
    state->camera.zoom *= (1.0 + scroll * 0.05);
    Vector2 mouse_az = GetScreenToWorld2D(mouse, state->camera);

    state->camera.target.x += mouse_bz.x - mouse_az.x;
    state->camera.target.y += mouse_bz.y - mouse_az.y;
  }
}

void __render_flickers(state_t *state)
{
  for (int i = 0; i < 256; i++)
  {
    Vector2 *pos = &state->s_login_points_pos[i];
    Color *clr = &state->s_login_points_clr[i];
    DrawRectangle(pos->x * 8, pos->y * 8, 8, 8, *clr);
    clr->a -= GetRandomValue(1, 4);
    if (clr->a <= 8)
    {
      pos->x = GetRandomValue(0, state->width / 8);
      pos->y = GetRandomValue(0, state->height / 8);
      clr->a = GetRandomValue(128, 256);
    }
  }
}

