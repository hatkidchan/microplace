#include "pksender.h"
#include "logging.h"
#include "utils.h"
#include <assert.h>

size_t make_pk_s_inf(client_t *c, pk_s_inf_t pkt, uint8_t *dst, size_t lim)
{
  (void)c;
  assert(lim >= (1 + sizeof(pkt)));
  dst[0] = PK_S_INF;
  memcpy(&dst[1], &pkt, sizeof(pk_s_inf_t));
  return 1 + sizeof(pkt);
}

size_t make_pk_s_cdt(client_t *c, pk_s_cdt_t pkt, uint8_t *dst, size_t lim)
{
  assert(lim >= (1 + MAX_CDT_PKT_LENGTH + sizeof(pk_s_cdt_t) - sizeof(void*)));
  dst[0] = PK_S_CDT;
  switch (pkt.type)
  {
    case PK_CDT_UNCOMPRESSED:
      {
        size_t sz = c->world->info->chunk_width * c->world->info->chunk_height;
        memcpy(&dst[1], &pkt, sizeof(pk_s_cdt_t) - sizeof(void *));
        memcpy(&dst[4], pkt.data, sz);
        return 1 + sizeof(pk_s_cdt_t) - sizeof(void *) + sz;
      }
      break;
    default:
      assert(0 && "Unsupported pk_s_cdt_t.type");
      break;
  }
  return 0;
}

size_t make_pk_s_cnt(client_t *c, pk_s_cnt_t pkt, uint8_t *dst, size_t lim)
{
  (void)c;
  assert(lim >= (1 + sizeof(pk_s_cnt_t)));
  dst[0] = PK_S_CNT;
  pkt._rsv[0] = 0x00;
  pkt._rsv[1] = 0x00;
  pkt._rsv[2] = 0x00;
  memcpy(&dst[1], &pkt, sizeof(pk_s_cnt_t));
  return 1 + sizeof(pk_s_cnt_t);
}

size_t make_pk_s_pix(client_t *c, pk_s_pix_t pkt, uint8_t *dst, size_t lim)
{
  (void)c;
  assert(lim >= (1 + sizeof(pk_s_pix_t)));
  dst[0] = PK_S_PIX;
  pkt._rsv = 0x00;
  memcpy(&dst[1], &pkt, sizeof(pk_s_pix_t));
  return 1 + sizeof(pk_s_pix_t);
}

size_t make_pk_s_msg(client_t *c, pk_s_msg_t pkt, uint8_t *dst, size_t lim)
{
  (void)c;
  assert(lim >= (1 + sizeof(pk_s_msg_t)));
  dst[0] = PK_S_MSG;
  memcpy(&dst[1], &pkt, sizeof(pk_s_msg_t));
  size_t sz = 4 + sizeof(uint64_t) + 1;
  switch (pkt.type)
  {
    case PK_MSG_PLAIN:
      {
        size_t len = strnlen(pkt.data, 256);
        strncpy((char *)&dst[sz], pkt.data, len);
        sz += len + 1;
      }
      break;
    case PK_MSG_COLOR:
    case PK_MSG_LOCALIZED:
      assert(0 && "Unsupported pk_s_msg_t.type");
      break;
  }
  return sz;
}

size_t make_pk_s_sys(client_t *c, pk_s_sys_t pkt, uint8_t *dst, size_t lim)
{
  (void)c;
  assert(pkt.length <= MAX_SYS_PKT_LENGTH);
  assert(lim >= (1 + sizeof(pkt.length) + MAX_SYS_PKT_LENGTH));
  dst[0] = PK_S_SYS;
  memcpy(&dst[1], &pkt.length, sizeof(pkt.length));
  memcpy(&dst[1 + sizeof(pkt.length)], pkt.data, pkt.length);
  return 1 + sizeof(pkt.length) + pkt.length;
}

void send_pk_s_inf(client_t *c, pk_s_inf_t pkt)
{
  uint8_t packet[1 + sizeof(pkt)] = { 0 };
  size_t len = make_pk_s_inf(c, pkt, packet, 1 + sizeof(pk_s_inf_t));
  mg_ws_send(c->mgconn, (const char *)packet, len, WEBSOCKET_OP_BINARY);
}

void send_pk_s_cdt(client_t *c, pk_s_cdt_t pkt)
{
  log_TRACE("Sending chunk (%d:%d) to %d", pkt.x, pkt.y, c->id);
  uint8_t buf[1 + sizeof(pk_s_cdt_t) - sizeof(void *) + MAX_CDT_PKT_LENGTH];
  size_t sz = make_pk_s_cdt(c, pkt, buf, sizeof(buf));
  loghex_TRACE(buf, sz);
  mg_ws_send(c->mgconn, (const char *)buf, sz, WEBSOCKET_OP_BINARY);
}

void send_pk_s_cnt(client_t *c, pk_s_cnt_t pkt)
{
  uint8_t packet[1 + sizeof(pk_s_cnt_t)] = { 0 };
  size_t sz = make_pk_s_cnt(c, pkt, packet, 1 + sizeof(pk_s_cnt_t));
  mg_ws_send(c->mgconn, (const char *)packet, sz, WEBSOCKET_OP_BINARY);
}

void send_pk_s_pix(client_t *c, pk_s_pix_t pkt)
{
  uint8_t packet[1 + sizeof(pkt)] = { 0 };
  size_t sz = make_pk_s_pix(c, pkt, packet, 1 + sizeof(pk_s_pix_t));
  mg_ws_send(c->mgconn, (const char *)packet, sz, WEBSOCKET_OP_BINARY);
}

void send_pk_s_msg(client_t *c, pk_s_msg_t pkt)
{
  uint8_t packet[1 + sizeof(pk_s_msg_t)] = { 0 };
  size_t sz = make_pk_s_msg(c, pkt, packet, 1 + sizeof(pk_s_msg_t));
  mg_ws_send(c->mgconn, (const char *)packet, sz, WEBSOCKET_OP_BINARY);
}

void send_pk_s_sys(client_t *c, pk_s_sys_t pkt)
{
  uint8_t packet[1 + sizeof(pkt.length) + MAX_SYS_PKT_LENGTH] = { 0 };
  size_t sz = make_pk_s_sys(c, pkt, packet, sizeof(packet));
  mg_ws_send(c->mgconn, (const char *)packet, sz, WEBSOCKET_OP_BINARY);
}

void send_s_message(client_t *c, uint8_t rgb[3], const char *message)
{
  pk_s_msg_t msg = {
    .timestamp = now_ms(),
    .r = rgb[0], .g = rgb[1], .b = rgb[2],
    .type = PK_MSG_PLAIN
  };
  memset(msg.data, 0, sizeof(msg.data));
  strcpy(msg.data, message);
  send_pk_s_msg(c, msg);
}

void send_s_kick(client_t *c, const char *reason)
{
  pk_s_msg_t msg = {
    .timestamp = now_ms(),
    .r = 0xff, .g = 0x00, .b = 0x00,
    .type = PK_MSG_PLAIN
  };
  strcpy(msg.data, reason);
  send_pk_s_msg(c, msg);
  c->mgconn->is_closing = true;
}

void send_s_chunk(client_t *c, uint8_t cx, uint8_t cy)
{
  uint8_t buf_chunkdata[65536] = { 0 };
  pk_s_cdt_t pkt = { .type = PK_CDT_UNCOMPRESSED, .x = cx, .y = cy };
  world_crop_chunk(c->world, cx, cy, buf_chunkdata);
  pkt.data = buf_chunkdata;
  send_pk_s_cdt(c, pkt);
}

void send_s_info(client_t *c)
{
  pk_s_inf_t pkt = {
    .cw = c->world->info->chunk_width,
    .ch = c->world->info->chunk_height,
    .nx = c->world->info->chunks_x,
    .ny = c->world->info->chunks_y,
  };
  strncpy(pkt.name, c->world->info->name, 128);
  strncpy(pkt.description, c->world->info->description, 256);
  send_pk_s_inf(c, pkt);
}

void send_s_counters(client_t *c)
{
  pk_s_cnt_t pkt = {
    .n_changes = c->world->info->n_changes,
    .n_connections = c->world->info->n_connections,
    .n_messages = c->world->info->n_messages,
    .online = c->server->n_clients,
    .timestamp = now_ms()
  };
  send_pk_s_cnt(c, pkt);
}

void send_s_bcast(client_t *head, void *pkt, size_t length)
{
  client_t *cur = head;
  while (cur != NULL)
  {
    mg_ws_send(cur->mgconn, (const char *)pkt, length, WEBSOCKET_OP_BINARY);
    cur = cur->next;
  }
}

void send_s_bcast_msg(client_t *head, uint8_t rgb[3], const char *message)
{
  pk_s_msg_t msg = {
    .timestamp = now_ms(),
    .r = rgb[0], .g = rgb[1], .b = rgb[2],
    .type = PK_MSG_PLAIN
  };
  memset(msg.data, 0, sizeof(msg.data));
  strcpy(msg.data, message);
  uint8_t packet[1 + sizeof(pk_s_msg_t)] = { 0 };
  size_t sz = make_pk_s_msg(head, msg, packet, 1 + sizeof(pk_s_msg_t));
  log_TRACE("Broadcasting message:");
  log_TRACE("type=%zd, rgb=(%d, %d, %d)", msg.type, msg.r, msg.g, msg.b);
  loghex_TRACE(packet, sz);
  send_s_bcast(head, packet, sz);
}

void send_s_bcast_cnt(client_t *head)
{
  pk_s_cnt_t pkt = {
    .n_changes = head->world->info->n_changes,
    .n_connections = head->world->info->n_connections,
    .n_messages = head->world->info->n_messages,
    .online = head->server->n_clients,
    .timestamp = now_ms()
  };
  uint8_t packet[1 + sizeof(pk_s_cnt_t)] = { 0 };
  size_t sz = make_pk_s_cnt(head, pkt, packet, 1 + sizeof(pk_s_cnt_t));
  log_TRACE("Broadcasting counters:");
  log_TRACE("changes=%zd", pkt.n_changes);
  log_TRACE("connections=%zd", pkt.n_connections);
  log_TRACE("messages=%zd", pkt.n_messages);
  log_TRACE("online=%zd", pkt.online);
  loghex_TRACE(packet, sz);
  send_s_bcast(head, packet, sz);
}
