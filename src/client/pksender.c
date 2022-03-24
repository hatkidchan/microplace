#include "pksender.h"
#include <assert.h>

size_t make_pk_c_set(pk_c_set_t pkt, uint8_t *dst, size_t lim)
{
  assert(lim >= (1 + sizeof(pk_c_set_t)));
  dst[0] = PK_C_SET;
  memcpy(&dst[1], &pkt, sizeof(pk_c_set_t));
  return 1 + sizeof(pk_c_set_t);
}


size_t make_pk_c_msg(pk_c_msg_t pkt, uint8_t *dst, size_t lim)
{
  assert(lim >= (1 + sizeof(pk_c_msg_t)));
  dst[0] = PK_C_MSG;
  strncpy((char *)&dst[1], pkt.text, 256);
  size_t ln = strnlen(pkt.text, 256);
  dst[ln + 2] = 0;
  return ln + 2;
}

size_t make_pk_c_crq(pk_c_crq_t pkt, uint8_t *dst, size_t lim)
{
  assert(lim >= (1 + sizeof(pk_c_crq_t)));
  dst[0] = PK_C_CRQ;
  memcpy(&dst[1], &pkt, sizeof(pk_c_crq_t));
  return 1 + sizeof(pk_c_crq_t);
}

size_t make_pk_c_sys(pk_c_sys_t pkt, uint8_t *dst, size_t lim)
{
  assert(lim >= (1 + sizeof(pk_c_sys_t) - sizeof(void *)));
  assert(pkt.length <= MAX_SYS_PKT_LENGTH);
  size_t sz = pkt.length + sizeof(uint32_t) + 1;
  memcpy(&dst[sizeof(uint32_t) + 1], pkt.data, pkt.length);
  return sz;
}

void send_pk_c_set(state_t *c, pk_c_set_t pkt)
{
  uint8_t packet[1 + sizeof(pkt)] = { 0 };
  size_t sz = make_pk_c_set(pkt, packet, sizeof(pk_c_set_t) + 1);
#ifdef PLATFORM_WEB
  emscripten_websocket_send_binary(c->wsock, packet, sz);
#else
  mg_ws_send(c->conn, (const char *)packet, sz, WEBSOCKET_OP_BINARY);
#endif
}

void send_pk_c_msg(state_t *c, pk_c_msg_t pkt)
{
  uint8_t packet[1 + sizeof(pkt)] = { 0 };
  size_t sz = make_pk_c_msg(pkt, packet, sizeof(pk_c_msg_t) + 1);
#ifdef PLATFORM_WEB
  emscripten_websocket_send_binary(c->wsock, packet, sz);
#else
  mg_ws_send(c->conn, (const char *)packet, sz, WEBSOCKET_OP_BINARY);
#endif
}

void send_pk_c_crq(state_t *c, pk_c_crq_t pkt)
{
  uint8_t packet[1 + sizeof(pkt)] = { 0 };
  size_t sz = make_pk_c_crq(pkt, packet, sizeof(pk_c_crq_t) + 1);
#ifdef PLATFORM_WEB
  emscripten_websocket_send_binary(c->wsock, packet, sz);
#else
  mg_ws_send(c->conn, (const char *)packet, sz, WEBSOCKET_OP_BINARY);
#endif
}

void send_pk_c_sys(state_t *c, pk_c_sys_t pkt)
{
  uint8_t packet[1 + sizeof(uint32_t) + MAX_SYS_PKT_LENGTH] = { 0 };
  size_t sz = make_pk_c_sys(pkt, packet,
      sizeof(uint32_t) + 1 + MAX_SYS_PKT_LENGTH);
#ifdef PLATFORM_WEB
  emscripten_websocket_send_binary(c->wsock, packet, sz);
#else
  mg_ws_send(c->conn, (const char *)packet, sz, WEBSOCKET_OP_BINARY);
#endif
}

