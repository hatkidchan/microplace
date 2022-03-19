#ifndef _CLIENT_STATE_H_
#define _CLIENT_STATE_H_

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

typedef enum {
  CLST_WHEREAMI,
  CLST_CONNECTING,
  CLST_CONNECTED,
  CLST_MAINLOOP,
  CLST_DISCONNECTED,
  CLST_RECONNECTING
} cl_state_en;

typedef void (*cli_message_handler)(void *fptr, void *data, size_t size);
typedef void (*cli_error_handler)(void *fptr, char *msg);

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
} state_t;

#endif
