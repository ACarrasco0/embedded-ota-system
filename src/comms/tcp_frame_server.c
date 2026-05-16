/*
* name: tcp_frame_server.c
* Description: This file contains the implementation of the TCP frame server.
* It includes functions for encoding and decoding frames according to a specific
* protocol, which consists of a Start of Frame (SOF), length, sequence number,
* command, data, and CRC. The encoding function takes care of escaping special
* characters, while the decoding function validates the frame structure and CRC.
*/
#include "tcp_frame_server.h"
#include <stdint.h>
#include <string.h>
#include "core/config.h"
#include "logger/logger.h"

static uint16_t frame_crc16_update(uint16_t crc, uint8_t data)
{
    crc ^= (uint16_t)data << 8;
    for (uint8_t i = 0; i < 8; i++) {
        if (crc & 0x8000) {
            crc = (crc << 1) ^ 0x1021;
        } else {
            crc <<= 1;
        }
    }
    return crc;
}

static uint16_t frame_crc_payload(uint16_t len,
                                  uint8_t seq,
                                  uint8_t cmd,
                                  const uint8_t *data)
{
    uint16_t crc = 0xFFFF;
    crc = frame_crc16_update(crc, (uint8_t)(len >> 8));
    crc = frame_crc16_update(crc, (uint8_t)(len & 0xFF));
    crc = frame_crc16_update(crc, seq);
    crc = frame_crc16_update(crc, cmd);
    for (uint16_t i = 0; i < len; i++) {
        crc = frame_crc16_update(crc, data[i]);
    }
    return crc;
}

static uint16_t frame_read_be16(const uint8_t *in)
{
    return ((uint16_t)in[0] << 8) | in[1];
}

static void frame_write_be16(uint8_t *out, uint16_t value)
{
    out[0] = (uint8_t)(value >> 8);
    out[1] = (uint8_t)(value & 0xFF);
}

int frame_encode(uint8_t cmd, uint8_t *data, uint16_t len, uint8_t *out)
{
    /*
    * Encodes a frame for transmission over TCP.
    * Returns the number of bytes written to the output buffer, or -1 on error.
    * It returns int instead of STATUS_E because it needs to return the length 
    * of the encoded frame and -1 on error, which is not compatible with the 
    * STATUS_E enum.
    */
    static uint8_t seq;
    if (out == NULL || len > FRAME_MAX_DATA || (len > 0 && data == NULL)) {
        return -1;
    }

    out[0] = FRAME_SOF;
    frame_write_be16(out + 1, len);
    out[3] = seq;
    out[4] = cmd;

    int pos = 5;
    for (uint16_t i = 0; i < len; i++) {
        uint8_t b = data[i];
        if (b == FRAME_SOF || b == FRAME_ESC) {
            if (pos + 2 > FRAME_MAX_BUFFER - 2) {
                return -1;
            }
            out[pos++] = FRAME_ESC;
        }
        if (pos + 1 > FRAME_MAX_BUFFER - 2) {
            return -1;
        }
        out[pos++] = b;
    }

    uint16_t crc = frame_crc_payload(len, seq, cmd, data);
    if (pos + 2 > FRAME_MAX_BUFFER) {
        return -1;
    }
    frame_write_be16(out + pos, crc);
    pos += 2;
    seq++;
    return pos;
}

static int frame_sync_to_sof(const uint8_t *buffer, int buffer_len, int *consumed)
{
    int i = 0;

    while (i < buffer_len && buffer[i] != FRAME_SOF) {
        i++;
    }

    if (i == buffer_len) {
        *consumed = buffer_len;
        return 0;
    }

    if (i > 0) {
        *consumed = i;
        return -1;
    }

    return 1;
}

static int frame_check_min_len(int buffer_len)
{
    // A valid frame must have at least 7 bytes:
    // SOF (1) + LEN (2) + SEQ (1) + CMD (1) + CRC (2)
    return (buffer_len >= 7);
}

static uint16_t frame_read_len(const uint8_t *buffer)
{
    return frame_read_be16(buffer + 1);
}

static int frame_decode_payload(const uint8_t *buffer,
                                int buffer_len,
                                int *pos,
                                uint16_t len,
                                frame_t *out_frame)
{
    int got = 0;
    while (got < len) {
        if (*pos >= buffer_len) {
            return -1;
        }

        uint8_t b = buffer[(*pos)++];
        if (b == FRAME_ESC) {
            if (*pos >= buffer_len) {
                return -1;
            }

            b = buffer[(*pos)++];

            if (b != FRAME_SOF && b != FRAME_ESC) {
                return -2;
            }
        }
        out_frame->data[got++] = b;
    }
    return got;
}

static int frame_check_crc(const uint8_t *buffer,
                           int pos,
                           uint16_t len,
                           frame_t *out_frame)
{
    uint16_t crc = frame_read_be16(buffer + pos);

    uint16_t expected = frame_crc_payload(len,
                                         buffer[3],
                                         buffer[4],
                                         out_frame->data);

    return (crc == expected);
}

STATUS_E frame_decode(uint8_t *buffer, int buffer_len, frame_t *out_frame,
                      int *consumed_bytes)
{
    if (!buffer || !out_frame || !consumed_bytes || buffer_len <= 0) {
        return STATUS_ERROR;
    }

    *consumed_bytes = 0;

    int sync = frame_sync_to_sof(buffer, buffer_len, consumed_bytes);
    if (sync == 0) return STATUS_INCOMPLETE;
    if (sync < 0) return STATUS_ERROR;

    if (!frame_check_min_len(buffer_len)) {
        return STATUS_INCOMPLETE;
    }

    uint16_t len = frame_read_len(buffer);
    
    int needed = 5 + len + 2; // header + payload + crc
    if (buffer_len < needed) {
        return STATUS_INCOMPLETE;
    }

    if (len > FRAME_MAX_DATA) {
        *consumed_bytes = 1;
        return STATUS_ERROR;
    }

    int pos = 5;

    int got = frame_decode_payload(buffer, buffer_len, &pos, len, out_frame);

    if (got == -1) return STATUS_INCOMPLETE;
    if (got == -2) {
        *consumed_bytes = 1;
        return STATUS_ERROR;
    }

    if (!frame_check_crc(buffer, pos, len, out_frame)) {
        *consumed_bytes = 1;
        return STATUS_ERROR;
    }

    pos += 2;

    out_frame->sof = FRAME_SOF;
    out_frame->len = len;
    out_frame->seq = buffer[3];
    out_frame->cmd = buffer[4];
    out_frame->crc = frame_read_be16(buffer + pos - 2);

    *consumed_bytes = pos;

    return STATUS_OK;
}