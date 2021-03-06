#include <math.h>
#include "../raylib/src/raylib.h"
#include <stddef.h>
#include "client/state.h"
#include "client/networking.h"
#include "client/pkhandlers.h"

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#else
// fuck windows.h
#ifdef _WIN32
#define _WINSOCK2API_
#define _WS2TCPIP_H_
#endif
#include "../mongoose/mongoose.h"
#endif

void main_loop(void *fnd);
int main(int argc, char **argv)
{
  state_t state = {
    .width = 640,
    .height = 480,
    .camera.zoom = 2.0,
    .camera.offset.x = 400,
    .camera.offset.y = 300,
    .camera.target.x = 400,
    .camera.target.y = 300,
    .camera.rotation = 0.0,
    .selected_pix = 0xaa,
    .colpick_open = false,
    .colpick_time = 0.0
  };
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(state.width, state.height, "microplace");
  SetWindowMinSize(state.width, state.height);
  SetTargetFPS(120);
  memset(&state.chat, 0, sizeof(state.chat));
  memset(&state.world, 0, sizeof(state.world));
  strncpy(state.server_address, "ws://hatkidchan.is-a.dev:8092/ws", 256);
  if (argc >= 2) strncpy(state.server_address, argv[1], 256);

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

void main_loop(void *fnd)
{
  state_t *state = (state_t *)fnd;
  if ((state->is_resized = IsWindowResized()))
  {
    state->width = GetScreenWidth();
    state->height = GetScreenHeight();
    state->camera.offset.x = state->width / 2.0;
    state->camera.offset.y = state->height / 2.0;
  }

  switch (state->state)
  {
    case CLST_BOOT:
      handle_state_boot(state);
      break;
    case CLST_LOGIN_SCREEN:
      handle_state_login_screen(state);
      break;
    case CLST_CONNECTING:
      handle_state_connecting(state);
      break;
    case CLST_EXCHANGING:
      handle_state_exchanging(state);
      break;
    case CLST_MAINLOOP:
      handle_state_mainloop(state);
      break;
    case CLST_CONNECTION_FAILED:
      handle_state_connection_failed(state);
      break;
    case CLST_RECONNECT_BEGIN:
      handle_state_reconnect_begin(state);
      break;
    case CLST_RECONNECT_WAIT:
      handle_state_reconnect_wait(state);
      break;
    case CLST_RECONNECT_FAILED:
      handle_state_reconnect_failed(state);
      break;
    case CLST_KICKED:
      handle_state_kicked(state);
      break;
  }
}
