#ifndef _S_PKSENDER_H_
#define _S_PKSENDER_H_

#include "srv_client_types.h"
#include "packets.h"

typedef void (*srv_pksender_t)(client_t *c, void *pkt);

size_t make_pk_s_inf(client_t *c, pk_s_inf_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_s_cdt(client_t *c, pk_s_cdt_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_s_cnt(client_t *c, pk_s_cnt_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_s_pix(client_t *c, pk_s_pix_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_s_msg(client_t *c, pk_s_msg_t pkt, uint8_t *dst, size_t lim);
size_t make_pk_s_sys(client_t *c, pk_s_sys_t pkt, uint8_t *dst, size_t lim);

void send_pk_s_inf(client_t *c, pk_s_inf_t pkt);
void send_pk_s_cdt(client_t *c, pk_s_cdt_t pkt);
void send_pk_s_cnt(client_t *c, pk_s_cnt_t pkt);
void send_pk_s_pix(client_t *c, pk_s_pix_t pkt);
void send_pk_s_msg(client_t *c, pk_s_msg_t pkt);
void send_pk_s_sys(client_t *c, pk_s_sys_t pkt);


// handy functions
void send_s_info(client_t *c);
void send_s_chunk(client_t *c, uint8_t cx, uint8_t cy);
void send_s_counters(client_t *c);
void send_s_message(client_t *c, uint8_t rgb[3], const char *message);
void send_s_kick(client_t *c, const char *reason);

void send_s_bcast(client_t *head, void *pkt, size_t length);
void send_s_bcast_msg(client_t *head, uint8_t rgb[3], const char *message);
void send_s_bcast_cnt(client_t *head);

#endif
