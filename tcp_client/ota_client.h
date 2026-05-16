#ifndef OTA_CLIENT_H
#define OTA_CLIENT_H

#include "core/config.h"

typedef enum {
    CMD_OTA_START = 0x01,
    CMD_OTA_DATA = 0x02,
    CMD_OTA_END = 0x03,
    CMD_PING = 0x10,
    CMD_ACK = 0x80,
    CMD_NACK = 0x81
} cmd_client_t;

int ota_client_send_update(const char *ip, const char *port, const char *path);

#endif /* OTA_CLIENT_H */
