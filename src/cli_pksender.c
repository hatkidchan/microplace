#include "cli_pksender.h"
#include "utils.h"
#include <assert.h>

void send_pk_c_set(struct mg_connection *c, pk_c_set_t pkt)
{
  uint8_t packet[1 + sizeof(pkt)] = { 0 };
  packet[0] = PK_C_SET;
  pkt._rsv = 0x00;
  memcpy(&packet[1], &pkt, sizeof(pkt));
  mg_ws_send(c, (const char *)packet, 1 + sizeof(pkt), WEBSOCKET_OP_BINARY);
}

void send_pk_c_msg(struct mg_connection *c, pk_c_msg_t pkt)
{
  uint8_t packet[1 + sizeof(pkt)] = { 0 };
  packet[0] = PK_C_MSG;
  strncpy((char *)&packet[1], pkt.text, 256);
  size_t ln = strnlen(pkt.text, 256);
  packet[ln + 2] = 0;
  mg_ws_send(c, (const char *)packet, ln + 2, WEBSOCKET_OP_BINARY);
}

void send_pk_c_crq(struct mg_connection *c, pk_c_crq_t pkt)
{
  uint8_t packet[1 + sizeof(pkt)] = { 0 };
  packet[0] = PK_C_CRQ;
  memcpy(&packet[1], &pkt, sizeof(pkt));
  mg_ws_send(c, (const char *)packet, 1 + sizeof(pkt), WEBSOCKET_OP_BINARY);
}

void send_pk_c_sys(struct mg_connection *c, pk_c_sys_t pkt)
{
  assert(pkt.length <= MAX_SYS_PACKET_LENGTH);
  uint8_t packet[1 + sizeof(uint32_t) + MAX_SYS_PACKET_LENGTH] = { 0 };
  size_t sz = pkt.length + sizeof(uint32_t) + 1;
  memcpy(&packet[sizeof(uint32_t) + 1], pkt.data, pkt.length);
  mg_ws_send(c, (const char *)packet, sz, WEBSOCKET_OP_BINARY);
}

