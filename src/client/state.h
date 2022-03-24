#ifndef _CLIENT_STATE_H_
#define _CLIENT_STATE_H_

#include "../../raylib/src/raylib.h"
#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#else
// fuck windows.h
#ifdef _WIN32
#define _WINSOCK2API_
#define _WS2TCPIP_H_
#endif
#include "../../mongoose/mongoose.h"
#endif

typedef enum {
  CLST_BOOT,
  CLST_LOGIN_SCREEN,
  CLST_CONNECTING,
  CLST_EXCHANGING,
  CLST_MAINLOOP,
  CLST_CONNECTION_FAILED,
  CLST_RECONNECT_BEGIN,
  CLST_RECONNECT_WAIT,
  CLST_RECONNECT_FAILED,
  CLST_KICKED,
} cl_state_en;

typedef void (*cli_message_handler)(void *fptr, void *data, size_t size);
typedef void (*cli_error_handler)(void *fptr, char *msg);

typedef struct {
  Color color;
  char text[256];
  float phase;
} chatmessage_t;

typedef struct {
  uint8_t cw, ch, nx, ny;
  int width, height;
  char name[128];
  char description[256];
  bool ready;
  bool has_chunk[256*256];
} worldinfo_t;

typedef struct state_s {
  uint64_t frame;
#ifdef PLATFORM_WEB
  EMSCRIPTEN_WEBSOCKET_T wsock;
#else
  struct mg_connection *conn;
  struct mg_mgr *manager;
#endif
  bool sock_connected;
  bool sock_dropped;

  cli_message_handler on_message;
  cli_error_handler on_error;

  cl_state_en state;
  int animation_frames_left;
  int timer_started_frame;

  char server_address[256];
  bool server_address_ed;
  chatmessage_t chat[16];
  worldinfo_t world;
  uint8_t selected_pix;
  
  Vector2 s_login_points_pos[256];
  Color s_login_points_clr[256];
  
  RenderTexture2D canvas;
  Camera2D camera;
  Vector2 c_pan_start, c_pan_src_target;
  int width, height;
  bool is_resized;
} state_t;

void handle_state_boot(state_t *state);
void handle_state_login_screen(state_t *state);
void handle_state_connecting(state_t *state);
void handle_state_exchanging(state_t *state);
void handle_state_mainloop(state_t *state);
void handle_state_connection_failed(state_t *state);
void handle_state_reconnect_begin(state_t *state);
void handle_state_reconnect_wait(state_t *state);
void handle_state_reconnect_failed(state_t *state);
void handle_state_kicked(state_t *state);

#endif
