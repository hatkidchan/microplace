#include "packets.h"
#include "srv_handlers.h"
#include "srv_pksender.h"
#include "utils.h"
#include "options.h"
#include <assert.h>

#define _CHECK_PK_SIZE(client, recved, expected) \
{ \
  if (recved < expected) \
  { \
    send_s_message(client, (uint8_t[3]){ 255, 0, 0 }, \
        "Incomplete packet");\
    return; \
  }\
}


client_t *__sh_find_client(server_t *srv, uint64_t id)
{
  client_t *cur = srv->clients_head;
  while (cur != NULL)
  {
    if (cur != NULL && cur->id == id)
      return cur;
    cur = cur->next;
  }
  return NULL;
}

void __sh_remove_client(server_t *server, client_t *client)
{
  if (server->clients_head == NULL)
    return;
  client_t *cur = server->clients_head, *prev = NULL;
  if (cur == NULL)
    return;
  while (cur != client)
  {
    if (cur->next == NULL)
    {
      return;
    }
    else
    {
      prev = cur;
      cur = cur->next;
    }
  }

  if (cur == server->clients_head)
  {
    server->clients_head = server->clients_head->next;
  }
  else
  {
    prev->next = cur->next;
  }

  mg_ws_send(client->mgconn, "", 0, WEBSOCKET_OP_CLOSE);
  printf("DISCONNECT %s\n", cur->address);
  free(cur);
  server->n_clients--;
}

void handle_mongoose(struct mg_connection *c, int et, void *edt, void *fdt)
{
  server_t *server = (server_t *)fdt;
  if (et != MG_EV_POLL)
    printf("MG_EVENT: %d (%s)\n", et, mg_ev_str(et));
  switch (et)
  {
    case MG_EV_HTTP_MSG:
      {
        struct mg_http_message *hm = (struct mg_http_message *)edt;
        if (mg_http_match_uri(hm, "/ws"))
        {
          mg_ws_upgrade(c, hm, NULL);
        }
      }
      break;
    case MG_EV_WS_OPEN:
      ws_on_open(server, c);
      break;
    case MG_EV_WS_MSG:
      {
        struct mg_ws_message *wm = (struct mg_ws_message *)edt;
        client_t *cli = __sh_find_client(server, c->id);
        assert(cli != NULL);
        ws_on_msg(server, cli, (void *)wm->data.ptr, wm->data.len);
      }
      break;
    case MG_EV_CLOSE:
      ws_on_close(server, c);
      break;
  }
}

void ws_on_open(server_t *srv, struct mg_connection *conn)
{
  client_t *client = calloc(1, sizeof(client_t));
  client->id = conn->id;
  client->mgconn = conn;
  client->world = srv->world;
  client->server = srv;
  memset(client->username, 0, 128);
  mg_straddr(conn, client->address, 256);
  client->next = srv->clients_head;
  srv->clients_head = client;
  srv->n_clients++;
  
  send_s_info(client);
  
  char tmp[256] = { 0 };
  snprintf(tmp, 256, "-!-=*=*=*=*=-!-");
  send_s_message(client, (uint8_t[3]){ 0, 255, 0 }, tmp);
  snprintf(tmp, 256, "Welcome to the %s server!", srv->world->info->name);
  send_s_message(client, (uint8_t[3]){ 0, 255, 0 }, tmp);
  snprintf(tmp, 256, "Online: %d", srv->n_clients);
  send_s_message(client, (uint8_t[3]){ 0, 255, 0 }, tmp);
  snprintf(tmp, 256, "NOTE: at this time, world is NOT protected");
  send_s_message(client, (uint8_t[3]){ 255, 0, 0 }, tmp);
  snprintf(tmp, 256, "Total world changes: %zd", srv->world->info->n_changes);
  send_s_message(client, (uint8_t[3]){ 0, 255, 0 }, tmp);
  snprintf(tmp, 256, "-!-=*=*=*=*=-!-");
  send_s_message(client, (uint8_t[3]){ 0, 255, 0 }, tmp);
  client->can_write = true;
  
  srv->world->info->n_connections++;
  snprintf(tmp, 256, " * %s joined", client->address);
  send_s_bcast_msg(srv->clients_head, (uint8_t[3]){ 77, 255, 77 }, tmp);
  send_s_bcast_cnt(srv->clients_head);
}

void ws_on_msg(server_t *srv, client_t *cli, void *pkt, size_t sz)
{
  (void)srv;
  (void)sz;
  uint8_t *in_pkt = (uint8_t *)pkt;
  pkt_type_en pkt_type = *in_pkt++;
  switch (pkt_type)
  {
    case PK_C_SET:
      {
        _CHECK_PK_SIZE(cli, sz, 1 + sizeof(pk_c_set_t));
        pk_c_set_t pkt = *(pk_c_set_t*)in_pkt;
        if (!cli->can_write)
        {
          send_s_message(cli, (uint8_t[3]){ 255, 0, 0 },
              "You cannot place pixels.");
          return;
        }
        if (!world_set(cli->world, pkt.x, pkt.y, pkt.val))
        {
          send_s_message(cli, (uint8_t[3]){ 255, 0, 0 },
              "You cannot place pixels here");
          return;
        }
        
        uint64_t now = now_ms();
        if (now - cli->last_placed_pixel_ts <= UP_COOLDOWN_MS)
        {
          send_s_message(cli, (uint8_t[3]){ 255, 0, 0 },
              "WOAH WOAH SLOW DOWN BITCH COOLDOWN IS A THING YNOW THAT RIGHT?");
          return;
        }

        uint8_t buf[1 + sizeof(pk_s_pix_t)];
        pk_s_pix_t out_pkt = {
          ._rsv = 0x00, .x = pkt.x, .y = pkt.y, .val = pkt.val
        };
        size_t sz = make_pk_s_pix(cli, out_pkt, buf, 1 + sizeof(pk_s_pix_t));
        send_s_bcast(srv->clients_head, buf, sz);
        cli->last_placed_pixel_ts = now;
        srv->world->info->n_changes++;
      }
      break;
      
    case PK_C_MSG:
      {
        _CHECK_PK_SIZE(cli, sz, 1 + sizeof(pk_c_msg_t));
        pk_c_msg_t msg = *(pk_c_msg_t *)in_pkt;
        if (cli->username[0] == 0)
        {
          send_s_message(cli, (uint8_t[3]){ 255, 0, 0 },
              "You cannot send messages");
          return;
        }
        char message[1024];
        snprintf(message, 1024, "[%s] %s", cli->username, msg.text);
        uint8_t buf[1024 + sizeof(pk_s_msg_t) + 1];
        pk_s_msg_t pkt = {
          .r = 255, .g = 255, .b = 255, .timestamp = now_ms(),
          .type = PK_MSG_PLAIN
        };
        strncpy(pkt.data, message, sizeof(pkt.data));
        size_t sz = make_pk_s_msg(cli, pkt, buf, 1024 + 1 + sizeof(pk_s_msg_t));
        send_s_bcast(srv->clients_head, buf, sz);
      }
      break;

    case PK_C_CRQ:
      {
        _CHECK_PK_SIZE(cli, sz, 1 + sizeof(pk_c_crq_t));
        pk_c_crq_t request = *(pk_c_crq_t *)in_pkt;
        send_s_chunk(cli, request.cx, request.cy);
      }
      break;

    case PK_C_DIS:
      {
        send_s_kick(cli, "Disconnected");
      }
      break;

    case PK_C_SYS:
      {
        pk_c_sys_t pkt;
        pkt.length = *(uint32_t *)&in_pkt[0];
        assert(pkt.length <= 65536);
        assert(0 && "PK_C_SYS is not supported yet.");
      }
      break;

    case PK_S_INF:
    case PK_S_CDT:
    case PK_S_CNT:
    case PK_S_PIX:
    case PK_S_MSG:
    case PK_S_SYS:
      send_s_kick(cli, "Invalid packet type");
      break;
    default:
      send_s_kick(cli, "Invalid packet type.");
      break;
  }
}

void ws_on_close(server_t *srv, struct mg_connection *conn)
{
  client_t *client = __sh_find_client(srv, conn->id);
  if (client != NULL)
  {
    __sh_remove_client(srv, client);
  }
}

