#include "tcp_server_task.h"
#include "tcp_server.h"
#include "tcp_frame_server.h"
#include "tcp_protocol_server.h"
#include "ota/ota.h"
#include "logger/logger.h"
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdint.h>
#include <errno.h> 

extern volatile uint8_t tcp_server_task_running;

static void tcp_server_default_protocol_handler(int client_fd, frame_t *frame)
{
    ota_ops_t ota = ota_get_ops();
    protocol_handler(client_fd, frame, &ota);
}

void tcp_server_task_with_handler(protocol_handler_t protocol_handler)
{
    if (!tcp_server_task_running) {
        return;
    }

    if (protocol_handler == NULL) {
        return;
    }

    static int client_fd = -1;
    static struct sockaddr_storage client_addr;
    static socklen_t addr_size;

    static uint8_t recv_buf[1024];
    static uint8_t pending_buf[1024];
    static int pending_len = 0;

    frame_t frame;
    int consumed = 0;

    /* accept client */
    if (client_fd < 0) {

        addr_size = sizeof(client_addr);
        client_fd = tcp_server_accept(&client_addr, &addr_size);

        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }

            if (!tcp_server_task_running) {
                return;
            }

            log_error("accept failed: %s", strerror(errno));
            return;
        }

        log_info("client connected");
        pending_len = 0;
    }

    /* recv */
    int bytes = tcp_server_recv(client_fd, (char *)recv_buf, sizeof(recv_buf));

    if (bytes > 0) {

        if (pending_len + bytes > (int)sizeof(pending_buf)) {
            log_error("buffer overflow, dropping client");
            close(client_fd);
            client_fd = -1;
            pending_len = 0;
            return;
        }

        memcpy(pending_buf + pending_len, recv_buf, bytes);
        pending_len += bytes;

        log_debug("pending_len=%d", pending_len);

        while (pending_len > 0) {

            STATUS_E result = frame_decode(
                pending_buf,
                pending_len,
                &frame,
                &consumed
            );

            if (result == STATUS_OK) {

                protocol_handler(client_fd, &frame);

                if (consumed <= 0 || consumed > pending_len) {
                    pending_len = 0;
                    break;
                }

                memmove(pending_buf,
                        pending_buf + consumed,
                        pending_len - consumed);

                pending_len -= consumed;
                continue;
            }

            if (result == STATUS_ERROR) {

                int drop = (pending_len > 0) ? 1 : 0;

                memmove(pending_buf,
                        pending_buf + drop,
                        pending_len - drop);

                pending_len -= drop;
                continue;
            }

            if (result == STATUS_INCOMPLETE) {
                break;
            }
        }

    } else if (bytes == 0) {

        log_info("client disconnected");
        close(client_fd);
        client_fd = -1;
        pending_len = 0;

    } else {

        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        if (errno == EINTR) return;

        log_error("recv error: %s", strerror(errno));

        close(client_fd);
        client_fd = -1;
        pending_len = 0;
    }
}

void tcp_server_task(void)
{
    tcp_server_task_with_handler(tcp_server_default_protocol_handler);
}
