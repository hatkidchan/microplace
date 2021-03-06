#include "networking.h"
#include "pkhandlers.h"
#include <stdio.h>

#ifdef PLATFORM_WEB
EM_BOOL ws_on_open(int et, const EmscriptenWebSocketOpenEvent *ev, void *s);
EM_BOOL ws_on_close(int et, const EmscriptenWebSocketCloseEvent *ev, void *s);
EM_BOOL ws_on_message(int et, const EmscriptenWebSocketMessageEvent *ev, void *s);
EM_BOOL ws_on_error(int et, const EmscriptenWebSocketErrorEvent *ev, void *s);
#else
void ws_handler(struct mg_connection *c, int ev, void *evd, void *fnd);
#endif

void cnet_connect(state_t *state, const char *addr)
{
  printf("Connecting to %s...\n", addr);
#ifdef PLATFORM_WEB
  EmscriptenWebSocketCreateAttributes wsattr = {
    .url = addr, .protocols = NULL, .createOnMainThread = true
  };
  EMSCRIPTEN_WEBSOCKET_T wsock = emscripten_websocket_new(&wsattr);
  state->wsock = wsock;
  emscripten_websocket_set_onopen_callback_on_thread(wsock, state, ws_on_open,
      EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD);
  emscripten_websocket_set_onclose_callback_on_thread(wsock, state, ws_on_close,
      EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD);
  emscripten_websocket_set_onerror_callback_on_thread(wsock, state, ws_on_error,
      EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD);
  emscripten_websocket_set_onmessage_callback_on_thread(wsock, state,
      ws_on_message, EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD);
#else
  state->conn = mg_ws_connect(state->manager, addr, ws_handler, state, NULL);
#endif
}

bool cnet_is_connected(state_t *state)
{
  return state->sock_connected
    && !state->sock_dropped
#ifdef PLATFORM_WEB
    && true
#else
    && state->conn != NULL
#endif
    ;
}

#ifdef PLATFORM_WEB

EM_BOOL ws_on_open(int et, const EmscriptenWebSocketOpenEvent *ev, void *s)
{
  state_t *state = (state_t *)s;
  state->sock_connected = true;
  state->sock_dropped = false;
  return true;
}

EM_BOOL ws_on_close(int et, const EmscriptenWebSocketCloseEvent *ev, void *s)
{
  state_t *state = (state_t *)s;
  state->sock_connected = false;
  state->sock_dropped = true;
  return true;
}

EM_BOOL ws_on_message(int et, const EmscriptenWebSocketMessageEvent *ev, void *s)
{
  state_t *state = (state_t *)s;
  on_message(s, (void *)ev->data, ev->numBytes);
  return true;
}

EM_BOOL ws_on_error(int et, const EmscriptenWebSocketErrorEvent *ev, void *s)
{
  state_t *state = (state_t *)s;
  on_error(s, "Unknown error");
  state->sock_connected = false;
  state->sock_dropped = true;
  state->wsock = 0;
  return true;
}

#else

void ws_handler(struct mg_connection *c, int ev, void *evd, void *fnd)
{
  (void)c;
  state_t *state = (state_t *)fnd;
  switch (ev)
  {
    case MG_EV_ERROR:
      printf("MG_EV_ERROR: %s\n", (char*)evd);
      state->on_error(fnd, (char *)evd);
      state->sock_connected = false;
      state->sock_dropped = true;
      state->conn = NULL;
      break;
    case MG_EV_CLOSE:
      printf("MG_EV_CLOSE\n");
      state->sock_connected = false;
      state->sock_dropped = true;
      state->conn = NULL;
      break;
    case MG_EV_WS_OPEN:
      printf("MG_EV_WS_OPEN\n");
      state->sock_dropped = false;
      state->sock_connected = true;
      break;
    case MG_EV_WS_MSG:
      {
        struct mg_ws_message *wm = (struct mg_ws_message *)evd;
        printf("MG_EV_WS_MSG %zd long\n", wm->data.len);
        state->on_message(fnd, (void *)wm->data.ptr, wm->data.len);
      }
      break;
  }
}

#endif
