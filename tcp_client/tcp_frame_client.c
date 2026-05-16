/*
* name: tcp_frame_client.c
* Description: This file contains the implementation of the TCP frame client.
* It includes functions for encoding and decoding frames according to a specific
* protocol, which consists of a Start of Frame (SOF), length, sequence number,
* command, data, and CRC. The encoding function takes care of escaping special
* characters, while the decoding function validates the frame structure and CRC.
*/
#include "tcp_frame_client.h"
#include <stdint.h>
#include <string.h>

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
/*
static uint16_t frame_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc = frame_crc16_update(crc, data[i]);
    }
    return crc;
}
*/
static uint16_t frame_crc_payload(uint16_t len, uint8_t seq, uint8_t cmd,
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

int frame_decode(uint8_t *buffer, int buffer_len, frame_t *out_frame,
                    int *consumed_bytes)
{
    /*
    * Decodes a frame received over TCP.
    * Returns 0 on success, -1 on failure. On success, out_frame is filled with 
    * the decoded frame and consumed_bytes indicates how many bytes were 
    * consumed from the input buffer. On failure, consumed_bytes indicates how
    * many bytes were consumed before the error was detected. In case of
    * incomplete data, consumed_bytes is set to 0.
    */
    if (buffer == NULL || out_frame == NULL || consumed_bytes == NULL || buffer_len <= 0) {
        return -1;
    }

    int idx = 0;
    while (idx < buffer_len && buffer[idx] != FRAME_SOF) {
        idx++;
    }
    if (idx > 0) {
        *consumed_bytes = idx;
        return -1;
    }
    if (buffer_len < 7) {
        *consumed_bytes = 0;
        return 0;
    }

    uint16_t len = frame_read_be16(buffer + 1);
    if (len > FRAME_MAX_DATA) {
        *consumed_bytes = 1;
        return -1;
    }

    uint8_t seq = buffer[3];
    uint8_t cmd = buffer[4];
    idx = 5;
    uint16_t got = 0;

    while (got < len) {
        if (idx >= buffer_len - 2) {
            *consumed_bytes = 0;
            return 0;
        }
        uint8_t b = buffer[idx++];
        if (b == FRAME_ESC) {
            if (idx >= buffer_len - 2) {
                *consumed_bytes = 0;
                return 0;
            }
            b = buffer[idx++];
            if (b != FRAME_SOF && b != FRAME_ESC) {
                *consumed_bytes = 1;
                return -1;
            }
        }
        out_frame->data[got++] = b;
    }

    if (idx + 2 > buffer_len) {
        *consumed_bytes = 0;
        return 0;
    }

    uint16_t crc = frame_read_be16(buffer + idx);
    uint16_t expected = frame_crc_payload(len, seq, cmd, out_frame->data);
    if (crc != expected) {
        *consumed_bytes = 1;
        return -1;
    }

    out_frame->sof = FRAME_SOF;
    out_frame->len = len;
    out_frame->seq = seq;
    out_frame->cmd = cmd;
    out_frame->crc = crc;
    idx += 2;
    *consumed_bytes = idx;
    return 1;
}