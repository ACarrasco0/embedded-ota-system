/*
* name: ota.h
* Description: This file contains the declarations for the OTA (Over-The-Air)
* update functionality. 
*/

#ifndef OTA_H
#define OTA_H

#include "core/config.h"
#include "ota/ota_port.h"
#include <stdint.h>
#include <time.h>

typedef struct {
    ota_port_file_t *fp;
    uint32_t expected_size;
    uint32_t received_size;
    uint32_t crc_calculated;
    uint32_t crc_expected;
    time_t last_time;
    uint8_t error_count;
} ota_ctx_t;

typedef struct {
    uint32_t size;
    uint32_t crc;
} ota_header_t;

typedef struct {
    STATUS_E (*start)(uint8_t*, uint16_t);
    STATUS_E (*write)(uint8_t*, uint16_t);
    STATUS_E (*stop)(void);
    STATUS_E (*timeout)(int);
} ota_ops_t;

STATUS_E ota_start(uint8_t *data, uint16_t len);
STATUS_E ota_write_chunk(uint8_t *data, uint16_t len);
STATUS_E ota_stop(void);
STATUS_E ota_check_timeout(int timeout_sec);
ota_ops_t ota_get_ops(void);

#endif /* OTA_H */
