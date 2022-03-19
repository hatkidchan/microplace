#ifndef _PACKETS_H_
#define _PACKETS_H_

#include <stdint.h>

#define MAX_SYS_PKT_LENGTH 16777216
#define MAX_CDT_PKT_LENGTH 65536

typedef enum {
    PK_C_SET = 0x1A,
    PK_C_MSG = 0x1E,
    PK_C_CRQ = 0x1C,
    PK_C_DIS = 0x11,
    PK_C_SYS = 0x1F,
    
    PK_S_INF = 0x09,
    PK_S_CDT = 0x0C,
    PK_S_CNT = 0x02,
    PK_S_PIX = 0x0A,
    PK_S_MSG = 0x0E,
    PK_S_SYS = 0x0F,
} pkt_type_en;

typedef struct {
    uint16_t _rsv;
    uint8_t val;
    uint16_t x;
    uint16_t y;
} __attribute__((packed)) pk_c_set_t;

typedef struct {
    char text[256];
    uint8_t _null;
} __attribute__((packed)) pk_c_msg_t;

typedef struct {
    uint8_t cx;
    uint8_t cy;
} __attribute__((packed)) pk_c_crq_t;

typedef struct {
    uint32_t length;
    uint8_t* data;
} __attribute__((packed)) pk_c_sys_t;

typedef struct {
    uint8_t cw;
    uint8_t ch;
    uint8_t nx;
    uint8_t ny;
    char name[128];
    char description[256];
} __attribute__((packed)) pk_s_inf_t;

typedef struct {
    enum pk_s_cdt_type_en {
        PK_CDT_UNCOMPRESSED = 0x00
    } type;
    uint8_t x;
    uint8_t y;
    void* data;
} __attribute__((packed)) pk_s_cdt_t;

typedef struct {
    uint8_t _rsv[3];
    uint64_t timestamp;
    uint16_t online;
    uint64_t n_changes;
    uint64_t n_messages;
    uint64_t n_connections;
} __attribute__((packed)) pk_s_cnt_t;

typedef struct {
    uint16_t _rsv;
    uint8_t val;
    uint16_t x;
    uint16_t y;
} __attribute__((packed)) pk_s_pix_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint64_t timestamp;
    enum pk_s_msg_type_en {
        PK_MSG_PLAIN = 0x00,
        PK_MSG_COLOR = 0x01,
        PK_MSG_LOCALIZED = 0x02
    } type;
    char data[65536];
} __attribute__((packed)) pk_s_msg_t;

typedef struct {
    uint32_t length;
    uint8_t* data;
} __attribute__((packed)) pk_s_sys_t;

#endif
