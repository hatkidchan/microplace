#ifndef _C_PKSENDER_H_
#define _C_PKSENDER_H_

#include "../packets.h"
#include "state.h"
#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#else
#ifdef _WIN32
#define _WINSOCK2API_
#define _WS2TCPIP_H_
#endif
#include "../../mongoose/mongoose.h"
#endif

size_t make_pk_c_set(pk_c_set_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_c_msg(pk_c_msg_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_c_crq(pk_c_crq_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_c_sys(pk_c_sys_t pkt, uint8_t *dst, size_t lim);

void send_pk_c_set(state_t *c, pk_c_set_t pkt);
void send_pk_c_msg(state_t *c, pk_c_msg_t pkt);
void send_pk_c_crq(state_t *c, pk_c_crq_t pkt);
void send_pk_c_sys(state_t *c, pk_c_sys_t pkt);

#endif
