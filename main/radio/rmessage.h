#pragma once
#include <stdint.h>

#define RMESSAGE_MAX_DATA 32

typedef struct __attribute__((packed)) {
    uint8_t  dst;
    uint32_t num;
    uint8_t  type;
    uint8_t  len;
} rmsg_header_t;

typedef struct __attribute__((packed)) {
    rmsg_header_t hdr;
    uint8_t data[RMESSAGE_MAX_DATA];
    uint32_t crc;
} rmessage_t;

typedef struct __attribute__((packed)) {
    uint8_t dst;
    uint32_t num;
    uint32_t crc;
} rack_t;
