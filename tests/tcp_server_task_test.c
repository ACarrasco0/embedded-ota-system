/* 
* name: tcp_server_task_test.c
* Description:
* Basic integration-style tests for tcp_server_task(). These are AI generated.
* These tests use fake socket/network functions to validate:
* - client connection
* - frame reception
* - protocol dispatch
* - disconnect handling
*/

#include "../src/comms/tcp_server.h"
#include "../src/comms/tcp_frame_server.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>


volatile uint8_t tcp_server_task_running = 1;

static int fake_accept_called = 0;
static int fake_recv_called = 0;
static int fake_close_called = 0;
static int fake_protocol_called = 0;

static int fake_client_fd = 42;

static uint8_t fake_recv_buffer[1024];
static int fake_recv_len = 0;
static int fake_recv_result = 0;

// Fake dependecies

int tcp_server_accept(struct sockaddr_storage *addr, socklen_t *addr_len)
{
    (void)addr;
    (void)addr_len;

    fake_accept_called++;
    return fake_client_fd;
}

int tcp_server_recv(int fd, char *buf, int len)
{
    (void)fd;
    (void)len;

    fake_recv_called++;

    if (fake_recv_result > 0) {
        memcpy(buf, fake_recv_buffer, fake_recv_len);
    }

    return fake_recv_result;
}

void protocol_handler(int fd, frame_t *frame)
{
    (void)fd;

    fake_protocol_called++;

    assert(frame != NULL);
}

int close(int fd)
{
    (void)fd;

    fake_close_called++;
    return 0;
}

// loggers are stubbed out to avoid cluttering test output
void log_info(const char *fmt, ...)  { (void)fmt; }
void log_debug(const char *fmt, ...) { (void)fmt; }
void log_error(const char *fmt, ...) { (void)fmt; }

// Test helpers

static void reset_fakes(void)
{
    fake_accept_called = 0;
    fake_recv_called = 0;
    fake_close_called = 0;
    fake_protocol_called = 0;

    fake_recv_len = 0;
    fake_recv_result = 0;

    memset(fake_recv_buffer, 0, sizeof(fake_recv_buffer));
}

// Tests cases
void test_valid_frame_calls_protocol_handler(void)
{
    reset_fakes();

    uint8_t payload[] = {0x11, 0x22, 0x33};

    fake_recv_len = frame_encode(
        0x55,
        payload,
        sizeof(payload),
        fake_recv_buffer
    );

    fake_recv_result = fake_recv_len;

    tcp_server_task();

    assert(fake_accept_called == 1);
    assert(fake_recv_called == 1);
    assert(fake_protocol_called == 1);

    printf("test_valid_frame_calls_protocol_handler passed\n");
}

void test_disconnect_closes_client(void)
{
    reset_fakes();

    fake_recv_result = 0;

    tcp_server_task();

    assert(fake_close_called == 1);

    printf("test_disconnect_closes_client passed\n");
}

void test_recv_eagain_does_not_disconnect(void)
{
    reset_fakes();

    fake_recv_result = -1;
    errno = EAGAIN;

    tcp_server_task();

    assert(fake_close_called == 0);

    printf("test_recv_eagain_does_not_disconnect passed\n");
}

void test_invalid_frame_does_not_call_protocol(void)
{
    reset_fakes();

    fake_recv_buffer[0] = 0x00;
    fake_recv_buffer[1] = 0x01;
    fake_recv_buffer[2] = 0x02;

    fake_recv_len = 3;
    fake_recv_result = 3;

    tcp_server_task();

    assert(fake_protocol_called == 0);

    printf("test_invalid_frame_does_not_call_protocol passed\n");
}


int main(void)
{
    test_valid_frame_calls_protocol_handler();

    test_disconnect_closes_client();

    test_recv_eagain_does_not_disconnect();

    test_invalid_frame_does_not_call_protocol();

    printf("All tcp_server_task tests passed!\n");

    return 0;
}