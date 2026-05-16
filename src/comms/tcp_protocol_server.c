#include "tcp_protocol_server.h"
#include "tcp_frame_server.h"
#include "logger/logger.h"
#include "tcp_server.h"
#include <stdint.h>

static int ota_finished = 0;

static STATUS_E handle_ota_start(frame_t *frame, ota_ops_t *ota)
{
    if (ota->start(frame->data, frame->len) != STATUS_OK) {
        log_error("OTA_START failed");
        return STATUS_ERROR;
    }
    ota_finished = 0;
    log_debug("OTA_START received");
    return STATUS_OK;
}

static STATUS_E handle_ota_data(frame_t *frame, ota_ops_t *ota)
{
    if (ota->write(frame->data, frame->len) != STATUS_OK) {
        log_error("OTA_DATA failed");
        return STATUS_ERROR;
    }
    log_debug("OTA_DATA received, len=%d", frame->len);
    return STATUS_OK;
}

static STATUS_E handle_ota_end(frame_t *frame, ota_ops_t *ota)
{
    (void)frame;

    if (ota_finished) {
        return STATUS_OK;
    }

    if (ota->stop() != STATUS_OK) {
        log_error("OTA_END failed");
        ota_finished = 1;
        return STATUS_ERROR;
    }

    log_debug("OTA_END received");
    return STATUS_OK;
}

static STATUS_E handle_ping(frame_t *frame)
{
    (void)frame;
    log_debug("PING received");
    return STATUS_OK;
}

static void send_frame_response(int client_fd, uint8_t cmd, uint8_t seq)
{
    uint8_t buffer[FRAME_MAX_BUFFER];
    uint8_t payload[1] = { seq };

    int len = frame_encode(cmd, payload, 1, buffer);
    if (len <= 0) {
        log_error("Failed to encode response cmd=0x%02X seq=%d", cmd, seq);
        return;
    }

    tcp_server_send(client_fd, (const char *)buffer, len);
}

static void send_ack(int client_fd, uint8_t seq)
{
    log_debug("Sending ACK seq=%d", seq);
    send_frame_response(client_fd, CMD_ACK, seq);
}

static void send_nack(int client_fd, uint8_t seq)
{
    log_debug("Sending NACK seq=%d", seq);
    send_frame_response(client_fd, CMD_NACK, seq);
}

void protocol_handler(int client_fd, frame_t *frame, ota_ops_t *ota)
{
    STATUS_E status = STATUS_ERROR;

    switch (frame->cmd) {

        case CMD_OTA_START:
            status = handle_ota_start(frame, ota);
            break;

        case CMD_OTA_DATA:
            status = handle_ota_data(frame, ota);
            break;

        case CMD_OTA_END:
            status = handle_ota_end(frame, ota);
            break;

        case CMD_PING:
            status = handle_ping(frame);
            break;

        default:
            log_debug("Unknown command: 0x%02X seq=%d",
                      frame->cmd, frame->seq);
            status = STATUS_ERROR;
            break;
    }

    if (status == STATUS_OK) {
        send_ack(client_fd, frame->seq);
    } else {
        send_nack(client_fd, frame->seq);
    }
}

STATUS_E protocol_check_timeout(int timeout_sec, ota_ops_t *ota)
{
    return ota->timeout(timeout_sec);
}
