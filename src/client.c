#include <math.h>
#include <raylib.h>
#include <stddef.h>
#include "cli_state.h"
#include "cli_networking.h"

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#else
// fuck windows.h
#ifdef _WIN32
#define _WINSOCK2API_
#define _WS2TCPIP_H_
#endif
#include "mongoose.h"
#endif

void on_message(void *fnd, void *data, size_t size);
void on_error(void *fnd, char *msg);
void main_loop(void *fnd);
int main(void)
{
  InitWindow(800, 600, "microplace");
  SetTargetFPS(60);
  state_t state = { 0 };
#ifdef PLATFORM_WEB
  emscripten_set_main_loop_arg(main_loop, &state, 0, 1);
#else
  struct mg_mgr mg_manager;
  state.manager = &mg_manager;
  mg_mgr_init(state.manager);
  state.on_message = on_message;
  state.on_error = on_error;

  while (!WindowShouldClose())
  {
    main_loop(&state);
    if (state.conn)
      mg_mgr_poll(&mg_manager, 5);
  }
#endif
  return 0;
}

void on_message(void *fnd, void *data, size_t size)
{
  (void)fnd; (void)data; (void)size;
}

void on_error(void *fnd, char *msg)
{
  (void)fnd; (void)msg;
}

void main_loop(void *fnd)
{
  state_t *state = (state_t *)fnd;
  (void)state;
  BeginDrawing();
  {
    switch (state->state)
    {
      case CLST_WHEREAMI:
        ClearBackground(RED);
        DrawText("OOPS", 16, 16, 32, WHITE);
        state->state = CLST_CONNECTING;
        cnet_connect(state, "ws://192.168.88.241:8092/ws");
        break;
      case CLST_CONNECTING:
        ClearBackground(YELLOW);
        DrawText("CONNECTING TO THE SERVER...", 16, 16, 32, BLACK);
        if (cnet_is_connected(state))
        {
          state->state = CLST_CONNECTED;
          state->animation_frames_left = 30;
        }
        else if (state->sock_dropped)
        {
          state->state = CLST_DISCONNECTED;
          state->animation_frames_left = 30;
        }
        break;
      case CLST_CONNECTED:
        ClearBackground(WHITE);
        DrawText("CONNECTED!!!!", 16, 16, 32, GREEN);
        if ((state->animation_frames_left--) <= 0)
          state->state = CLST_MAINLOOP;
        break;
      case CLST_MAINLOOP:
        ClearBackground(WHITE);
        DrawText("mm yes yes", 16, 16, 32, BLACK);
        if (!cnet_is_connected(state))
          state->state = CLST_DISCONNECTED;
        break;
      case CLST_DISCONNECTED:
        ClearBackground(RED);
        DrawText("DISCONNECTED", 16, 16, 32, WHITE);
        state->animation_frames_left = 30;
        state->state = CLST_RECONNECTING;
        break;
      case CLST_RECONNECTING:
        if ((state->animation_frames_left--) <= 0)
          state->state = CLST_WHEREAMI;
        break;
    }
  }
  EndDrawing();
  state->frame++;
}
