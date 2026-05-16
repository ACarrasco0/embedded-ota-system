/*
* name: tcp_frame_server.h
* Description: This file contains the declarations for the TCP frame server
* functionality, including frame encoding and decoding functions.
* Frame Structure: [SOF][LEN(2)][SEQ][CMD][DATA...][CRC(2)]
*/
#ifndef TCP_FRAME_SERVER_H
#define TCP_FRAME_SERVER_H

#include "core/config.h"
#include <stdint.h>
#include "core/config.h"

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

STATUS_E frame_decode(uint8_t *buffer,
                 int buffer_len,
                 frame_t *out_frame,
                 int *consumed_bytes);

#endif /* TCP_FRAME_SERVER_H */
