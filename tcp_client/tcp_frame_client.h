#ifndef TCP_FRAME_CLIENT_H
#define TCP_FRAME_CLIENT_H

#include "core/config.h"
#include <stdint.h>

#define FRAME_MAX_BUFFER 512
#define FRAME_MAX_DATA 504
#define FRAME_SOF 0x7E
#define FRAME_ESC 0x7D

typedef struct {
    uint8_t sof;
    uint16_t len;
    uint8_t seq;
    uint8_t cmd;
    uint8_t data[FRAME_MAX_DATA];
    uint16_t crc;
} frame_t;
 
int frame_encode(uint8_t cmd,
                     uint8_t *data,
                     uint16_t len,
                     uint8_t *out);

int frame_decode(uint8_t *buffer,
                 int buffer_len,
                 frame_t *out_frame,
                 int *consumed_bytes);

#endif /* TCP_FRAME_CLIENT_H */