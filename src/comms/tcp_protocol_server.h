#ifndef TCP_PROTOCOL_SERVER_H
#define TCP_PROTOCOL_SERVER_H

#include "core/config.h"
#include "tcp_frame_server.h"
#include "ota/ota.h"

// Command definitions for the protocol
typedef enum {
    CMD_OTA_START = 0x01,
    CMD_OTA_DATA = 0x02,
    CMD_OTA_END = 0x03,
    CMD_PING = 0x10,
    CMD_ACK = 0x80,
    CMD_NACK = 0x81
} cmd_server_t;

// State machine (not sure what to do with this yet, but it will be useful for 
// the OTA process)

void protocol_handler(int client_fd, frame_t *frame, ota_ops_t *ota);
STATUS_E protocol_check_timeout(int timeout_sec, ota_ops_t *ota);

#endif /* TCP_PROTOCOL_SERVER_H */
