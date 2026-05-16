/* 
* name: tcp_frame_server_test.c
* Description: These test are made only for validating the frame encoding and 
* decoding logic. So this way I can be sure that the core framing logic is
* correct before I start developing the client part and adding more features and
* layers on top of it.
*/

#include "../src/comms/tcp_frame_server.h"
#include "../src/core/config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_encode_decode(void)
{
    uint8_t data[] = {0x01, 0x02, 0x03};
    uint8_t out[FRAME_MAX_BUFFER];
    frame_t decoded;
    int consumed;

    int len = frame_encode(0x10, data, sizeof(data), out);
    assert(len > 0);

    STATUS_E res = frame_decode(out, len, &decoded, &consumed);
    assert(res == STATUS_OK);
    assert(decoded.cmd == 0x10);
    assert(decoded.len == sizeof(data));
    assert(memcmp(decoded.data, data, sizeof(data)) == 0);
    printf("test_encode_decode passed\n");
} 

void test_escape(void)
{
    uint8_t data[] = {FRAME_SOF, 0x42, FRAME_ESC};
    uint8_t out[FRAME_MAX_BUFFER];
    frame_t decoded;
    int consumed;

    int len = frame_encode(0x20, data, sizeof(data), out);
    assert(len > 0);

    STATUS_E res = frame_decode(out, len, &decoded, &consumed);
    assert(res == STATUS_OK);
    assert(decoded.cmd == 0x20);
    assert(decoded.len == sizeof(data));
    assert(memcmp(decoded.data, data, sizeof(data)) == 0);
    printf("test_escape passed\n");
}

void test_zero_length_payload(void)
{
    uint8_t out[FRAME_MAX_BUFFER];
    frame_t decoded;
    int consumed;

    int len = frame_encode(0x33, NULL, 0, out);

    STATUS_E res = frame_decode(out, len, &decoded, &consumed);

    assert(res == STATUS_OK);
    assert(decoded.len == 0);

    printf("test_zero_length_payload passed\n");
}

void test_invalid_escape(void)
{
    uint8_t raw[] = {
        FRAME_SOF,
        0x00, 0x01,
        0x01,
        0x10,
        FRAME_ESC,
        0x55,
        0x00, 0x00
    };

    frame_t decoded;
    int consumed;

    STATUS_E res = frame_decode(raw, sizeof(raw), &decoded, &consumed);

    assert(res == STATUS_ERROR);

    printf("test_invalid_escape passed\n");
}

void test_multiple_frames(void)
{
    uint8_t data1[] = {0x01};
    uint8_t data2[] = {0x02};

    uint8_t buf[FRAME_MAX_BUFFER];

    frame_t decoded;
    int consumed;

    int len1 = frame_encode(0x10, data1, sizeof(data1), buf);
    int len2 = frame_encode(0x20, data2, sizeof(data2), buf + len1);

    STATUS_E res;

    res = frame_decode(buf, len1 + len2, &decoded, &consumed);

    assert(res == STATUS_OK);
    assert(decoded.cmd == 0x10);

    res = frame_decode(buf + consumed,
                       len1 + len2 - consumed,
                       &decoded,
                       &consumed);

    assert(res == STATUS_OK);
    assert(decoded.cmd == 0x20);

    printf("test_multiple_frames passed\n");
}

void test_garbage_before_frame(void)
{
    uint8_t raw[FRAME_MAX_BUFFER];
    frame_t decoded;
    int consumed;

    raw[0] = 0x00;
    raw[1] = 0x11;
    raw[2] = 0x22;
    raw[3] = FRAME_SOF;

    STATUS_E res = frame_decode(raw, 4, &decoded, &consumed);

    assert(res == STATUS_ERROR);
    assert(consumed == 3);

    printf("test_garbage_before_frame passed\n");
}

void test_invalid_crc(void)
{
    uint8_t data[] = {0xAA, 0xBB};
    uint8_t out[FRAME_MAX_BUFFER];
    frame_t decoded;
    int consumed;

    int len = frame_encode(0x11, data, sizeof(data), out);

    out[len - 1] ^= 0xFF;

    STATUS_E res = frame_decode(out, len, &decoded, &consumed);

    assert(res == STATUS_ERROR);
    assert(consumed > 0);

    printf("test_invalid_crc passed\n");
}

void test_incomplete_frame(void)
{
    uint8_t data[] = {0x01, 0x02, 0x03};
    uint8_t out[FRAME_MAX_BUFFER];
    frame_t decoded;
    int consumed;

    int len = frame_encode(0x10, data, sizeof(data), out);

    STATUS_E res = frame_decode(out, len - 2, &decoded, &consumed);

    assert(res == STATUS_INCOMPLETE);
    assert(consumed == 0);

    printf("test_incomplete_frame passed\n");
}

int main(void)
{
    test_encode_decode();
    test_escape();
    test_incomplete_frame();
    test_invalid_crc();
    test_garbage_before_frame();
    test_multiple_frames();
    test_invalid_escape();
    test_zero_length_payload();

    printf("All tests passed!\n");
    return STATUS_OK;
}