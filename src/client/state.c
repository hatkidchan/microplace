#include "state.h"
#include "utils.h"
#include "networking.h"
#include "../packets.h"
#include "pksender.h"
#include "../../raylib/src/raylib.h"
#include "../../raygui/src/raygui.h"
#include <math.h>

#define COLPICK_WIDTH 16

void __handle_pan_zoom(state_t *state);
void __render_flickers(state_t *state);
void __handle_colpick(state_t *state);
void __draw_color_circle(Vector2 center, int index);
void __draw_text_shadowed(const char *txt, int x, int y, int size, Color color);

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

    const struct {
      Color color;
      char text[128];
      int size;
    } lines[] = {
      { GRAY, "Controls:", 12 },
      { GRAY, "LMB Drag - pan", 12 },
      { GRAY, "Mouse wheel - zoom", 12 },
      { GRAY, "LMB double - place pixel", 12 },
      { GRAY, "RMB Hold/Click - color picker", 12 },
      { GRAY, "MMB Click - copy color (NOT WORKING YET)", 12 },
      { MAROON, "ESC - exit", 12 },
      { GRAY, "", 12 },
    };

    for (int i = 0; i < sizeof(lines) / sizeof(lines[0]); i++)
    {
      DrawText(lines[i].text, rec.x, rec.y, lines[i].size, lines[i].color);
      rec.y += lines[i].size;
    }
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
        DrawText(txt, mouse.x + ox, mouse.y + oy + 14, 14, BLACK);
      }
    }
    DrawText(txt, mouse.x, mouse.y + 14, 14, WHITE);

    for (int i = 0; i < 16; i++)
    {
      int y = state->height - (16 - i) * 20;
      chatmessage_t *msg = &state->chat[i];
      if (msg->phase <= 0.01) continue;

      float alpha = msg->phase >= 0.5 ? 1.0 : msg->phase / 0.5;

      __draw_text_shadowed(msg->text, 8, y, 20, Fade(msg->color, alpha));
      msg->phase *= 0.995;
    }

    __handle_colpick(state);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !state->colpick_open)
    {
      double delta = GetTime() - state->last_click;
      if (delta <= 0.3)
      {
        pk_c_set_t pkt = {
          .x = worldpos.x,
          .y = worldpos.y,
          .val = state->selected_pix
        };
        if (state->sock_connected)
          send_pk_c_set(state, pkt);
        state->last_click = GetTime() + delta * 2.0;
      }
      state->last_click = GetTime();
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
    if (state->sock_dropped || !state->sock_connected)
    {
      state->state = CLST_RECONNECT_BEGIN;
      state->sock_reconnect_attempts = 5;
    }
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
  if (state->sock_reconnect_attempts <= 0)
  {
    state->state = CLST_RECONNECT_FAILED;
    return;
  }
  state->sock_reconnect_attempts--;
  state->timer_started_frame = state->frame;
  state->world.ready = false;
  cnet_connect(state, state->server_address);
  state->state = CLST_RECONNECT_WAIT;
}

void handle_state_reconnect_wait(state_t *state)
{
  BeginDrawing();
  {
    ClearBackground(BLACK);
    __render_flickers(state);
    const char *text = TextFormat("Reconnecting... (%d attempts left)",
        state->sock_reconnect_attempts);
    int tw = MeasureText(text, 32);
    int x = (state->width - tw) / 2,
        y = (state->height - 32) / 2;
    DrawText(text, x, y, 32, GRAY);
    y += 32;

    int remaining_frames = state->frame - state->timer_started_frame;
    float progress = (float)remaining_frames / 240.0;
    if (progress >= 1.0)
      DrawLine(x + tw * (progress - 1.), y, x + tw, y, GRAY);
    else
      DrawLine(x, y, x + tw * progress, y, GRAY);


    if (state->frame >= state->timer_started_frame + 480)
      state->state = CLST_RECONNECT_BEGIN;
    else if (cnet_is_connected(state))
      state->state = CLST_EXCHANGING;
  }
  EndDrawing();
  state->frame++;
}

void handle_state_reconnect_failed(state_t *state)
{
  BeginDrawing();
  {
    ClearBackground(BLACK);

    const char *text = "Reconnecting failed";
    int tw = MeasureText(text, 32);
    int x = (state->width - tw) / 2,
        y = (state->height - 32) / 2;
    DrawText(text, x, y, 32, RED); y += 48;

    Rectangle rec = {
      .x = (state->width - 400) / 2., .y = y,
      .width = 400, .height = 40
    };
    if (GuiButton(rec, "Back to login screen"))
    {
      state->state = CLST_LOGIN_SCREEN;
      state->world.ready = false;
    }
  }
  EndDrawing();
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
  state->s_login_points_pos[0].x = (int)((GetMouseX() - 4.0) / 8.0);
  state->s_login_points_pos[0].y = (int)((GetMouseY() - 4.0) / 8.0);
  Vector2 delta = GetMouseDelta();
  float delta_f = sqrtf(delta.x * delta.x + delta.y * delta.y);
  if (delta_f >= 8.0)
  {
    state->s_login_points_clr[0].a = 255;
  }
}

void __draw_color_circle(Vector2 center, int ndx)
{
  for (int ring = 0, color = 0; ring < 8; ring++)
  {
    int n_colors = ring == 0 ? 16 : (ring == 7 ? 24 : 36);
    float r_inner = 24.0 + ring * COLPICK_WIDTH,
          r_outer = r_inner + COLPICK_WIDTH,
          step = 360.0 / (float)n_colors,
          angle = 0.0;
    for (int i = 0; i < n_colors; i++, angle += step, color++)
    {
      Color clr = Color256(color);
      DrawRing(center, r_inner, r_outer, angle, angle + step, 20, clr);
    }
  }

  if (ndx >= 0 && ndx <= 255)
  {
    int ring = ndx < 16 ? 0 : (ndx >= 232 ? 7 : ((ndx - 16) / 36) + 1),
        sector = ring == 0 ? ndx : (ring == 7 ? ndx - 232 : (ndx - 16) % 36),
        colors = ring == 0 ? 16 : (ring == 7 ? 24 : 36);
    float r_inner = 24.0 + ring * COLPICK_WIDTH,
          r_outer = r_inner + COLPICK_WIDTH,
          step = 360.0 / (float)colors, angle = sector * step;

    DrawRingLines(center, r_inner, r_outer, 0, 360, 90, WHITE);
    DrawRingLines(center, r_inner, r_outer, angle, angle + step, 32, WHITE);
  }
}

void __handle_colpick(state_t *state)
{
  Vector2 mouse = GetMousePosition();

  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !state->colpick_open)
  {
    state->colpick_center.x = mouse.x;
    state->colpick_center.y = mouse.y;
    state->colpick_open = true;
    state->colpick_time = GetTime();
  }

  if ((IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)
        || (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && state->colpick_open))
      && (GetTime() - state->colpick_time) >= 0.3)
  {
    state->colpick_open = false;
  }

  if (state->colpick_open)
  {
    Vector2 direction = { mouse.x - state->colpick_center.x,
      mouse.y - state->colpick_center.y };
    float distance = sqrtf(powf(direction.x, 2.0) + powf(direction.y, 2.0));
    float angle = atan2f(-direction.y, direction.x) * 180.0 / M_PI + 90.0;

    if (distance >= 24.0 && distance <= (24.0 + 8.0 * COLPICK_WIDTH))
    {
      int ring = (distance - 24.0) / COLPICK_WIDTH,
          n_colors = ring == 0 ? 16 : (ring == 7 ? 24 : 36),
          start = ring == 0 ? 0 : (ring == 7 ? 232 : ((ring - 1) * 36 + 16)),
          sector = (int)floorf((angle + 360.0) * n_colors / 360.0) % n_colors;
      state->selected_pix = start + sector;
    }

    __draw_color_circle(state->colpick_center, state->selected_pix);
  }
}

void __draw_text_shadowed(const char *txt, int x, int y, int size, Color color)
{
  for (int oy = -1; oy <= 1; oy++)
  {
    for (int ox = -1; ox <= 1; ox++)
    {
      DrawText(txt, 8 + ox, y + oy, 20, Fade(BLACK, color.a / 255.0));
    }
  }
  DrawText(txt, 8, y, 20, color);
}
