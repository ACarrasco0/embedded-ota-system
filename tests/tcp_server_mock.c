/*
* name: tcp_server_mock.c
* Description: Mock for the TCP server send function used by protocol tests.
*/

#include "comms/tcp_frame_server.h"
#include <string.h>

static int mock_send_fd;
static int mock_send_count;
static int mock_send_len;
static uint8_t mock_send_buffer[FRAME_MAX_BUFFER];

void tcp_server_mock_reset(void)
{
    mock_send_fd = -1;
    mock_send_count = 0;
    mock_send_len = 0;
    memset(mock_send_buffer, 0, sizeof(mock_send_buffer));
}

int tcp_server_mock_send_count(void)
{
    return mock_send_count;
}

int tcp_server_mock_send_fd(void)
{
    return mock_send_fd;
}

int tcp_server_mock_send_len(void)
{
    return mock_send_len;
}

uint8_t *tcp_server_mock_send_buffer(void)
{
    return mock_send_buffer;
}

int tcp_server_send(int client_fd, const char *buffer, int bytes)
{
    mock_send_fd = client_fd;
    mock_send_count++;
    mock_send_len = bytes;

    if (bytes > FRAME_MAX_BUFFER) {
        bytes = FRAME_MAX_BUFFER;
    }

    memcpy(mock_send_buffer, buffer, bytes);
    return bytes;
}
