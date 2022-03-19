#ifndef _C_PKSENDER_H_
#define _C_PKSENDER_H_

#include "packets.h"
#include "mongoose.h"

size_t make_pk_c_set(pk_c_set_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_c_msg(pk_c_msg_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_c_crq(pk_c_crq_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_c_sys(pk_c_sys_t pkt, uint8_t *dst, size_t lim);

void send_pk_c_set(struct mg_connection *c, pk_c_set_t pkt);
void send_pk_c_msg(struct mg_connection *c, pk_c_msg_t pkt);
void send_pk_c_crq(struct mg_connection *c, pk_c_crq_t pkt);
void send_pk_c_sys(struct mg_connection *c, pk_c_sys_t pkt);

#endif
