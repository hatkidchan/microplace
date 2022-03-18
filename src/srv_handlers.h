#ifndef _SRV_HANDLERS_H_
#define _SRV_HANDLERS_H_

#include "mongoose.h"
#include "srv_state.h"
#include "srv_client_types.h"

void handle_mongoose(struct mg_connection *c, int et, void *edt, void *fdt);
void ws_on_open(server_t *srv, struct mg_connection *conn);
void ws_on_msg(server_t *srv, client_t *cli, void *pkt, size_t sz);
void ws_on_close(server_t *srv, struct mg_connection *conn);

#endif
