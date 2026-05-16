
/*
* name: ota.c
* Description: This file contains the implementation of the OTA (Over-The-Air)
* update functionality. It download a new binary and stores it in a temporary 
* location /opt/my_app/upfate/my_app.new and once it is validated it replaces
* the current binary /usr/local/bin/my_app with the new one. 
*/

#include "ota.h"
#include "logger/logger.h"
#include <stdint.h>
#include <string.h>

#define OTA_TEMP_PATH "/opt/my_app/update/my_app.new"
#define OTA_FINAL_PATH "/opt/my_app/current/my_app"
#define OTA_BACKUP_PATH "/opt/my_app/backup/my_app.bak"
#define OTA_HEADER_SIZE 8
#define MAX_FW_SIZE (10 * 1024 * 1024)

/* OTA context */
static ota_ctx_t ctx = {
    .fp = NULL,
    .expected_size = 0,
    .received_size = 0,
    .crc_calculated = 0,
    .crc_expected = 0,
    .last_time = 0,
    .error_count = 0
};

// Expose the operations through a function.
static ota_ops_t ops = {
    .start = ota_start,
    .write = ota_write_chunk,
    .stop = ota_stop,
    .timeout = ota_check_timeout
};


static uint32_t ota_crc32_update(uint32_t crc, const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320u;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static uint32_t ota_read_be32(const uint8_t *buffer)
{
    return ((uint32_t)buffer[0] << 24) |
           ((uint32_t)buffer[1] << 16) |
           ((uint32_t)buffer[2] << 8) |
            (uint32_t)buffer[3];
}

static STATUS_E ota_apply_update(void)
{
    /* Rename the current binary to backup, then rename the new binary to the 
    * final path. If any step fails, try to restore the original binary. If the
    * backup doesn't exist, just remove the new binary. This way we ensure that
    * we don't leave the system without a valid binary in case of failure.
    */
    log_info("OTA apply: replacing current binary");

    ota_port_remove(OTA_BACKUP_PATH);
    if (ota_port_rename(OTA_FINAL_PATH, OTA_BACKUP_PATH) != 0) {
        log_error("rename backup failed: %s", ota_port_last_error());
        if (!ota_port_error_is_not_found()) {
            return STATUS_ERROR;
        }
    }
    else {
        log_info("OTA apply: backup created");
    }

    if (ota_port_rename(OTA_TEMP_PATH, OTA_FINAL_PATH) != 0) {
        log_error("rename new file failed: %s", ota_port_last_error());
        if (!ota_port_error_is_not_found()) {
            ota_port_rename(OTA_BACKUP_PATH, OTA_FINAL_PATH);
        }
        return STATUS_ERROR;
    }
    log_info("OTA apply: new binary installed");

    if (ota_port_chmod(OTA_FINAL_PATH, 0755) != 0) {
        log_error("chmod failed: %s", ota_port_last_error());
        ota_port_rename(OTA_FINAL_PATH, OTA_TEMP_PATH);
        ota_port_rename(OTA_BACKUP_PATH, OTA_FINAL_PATH);
        return STATUS_ERROR;
    }
    log_info("OTA apply: permissions updated");

    return STATUS_OK;
}

STATUS_E ota_start(uint8_t *data, uint16_t len)
{
    /*
    * Creates a new OTA context and opens the temporary file for writing. It 
    * reads the expected size and CRC from the header data. Rejects if OTA already in progress.
    */
    if (data == NULL || len < OTA_HEADER_SIZE) {
        return STATUS_INVALID_PARAM;
    }

    if (ctx.fp != NULL) {
        return STATUS_ERROR;
    }

    uint32_t expected_size = ota_read_be32(data);
    if (expected_size == 0 || expected_size > MAX_FW_SIZE) {
        return STATUS_INVALID_PARAM;
    }
    
    ctx.expected_size = expected_size;
    ctx.crc_expected = ota_read_be32(data + 4);
    ctx.received_size = 0;
    ctx.crc_calculated = 0xFFFFFFFFu;
    ctx.last_time = ota_port_time();
    ctx.error_count = 0;

    /* Create the OTA update directory if it doesn't exist */
    log_debug("OTA path = %s", OTA_TEMP_PATH);

    if (ota_port_mkdir_parent(OTA_TEMP_PATH) != 0) {
        log_error("Failed to create OTA directory for %s: %s",
                  OTA_TEMP_PATH, ota_port_last_error());
        memset(&ctx, 0, sizeof(ctx));
        return STATUS_ERROR;
    }
    
    ctx.fp = ota_port_open(OTA_TEMP_PATH, "wb");
    if (ctx.fp == NULL) {
        log_error("Failed to open OTA temp file %s: %s",
                  OTA_TEMP_PATH, ota_port_last_error());
        memset(&ctx, 0, sizeof(ctx));
        return STATUS_ERROR;
    }

    log_debug("OTA session started: expected_size=%u, crc=0x%08X", 
              ctx.expected_size, ctx.crc_expected);
    return STATUS_OK;
}

STATUS_E ota_write_chunk(uint8_t *data, uint16_t len)
{
    /*
    * Writes a chunk of data to the temporary file and updates the CRC.
    */
    if (ctx.fp == NULL || data == NULL || len == 0) {
        return STATUS_INVALID_PARAM;
    }

    if (ctx.received_size + len > ctx.expected_size) {
        ota_port_close(ctx.fp);
        ctx.fp = NULL;
        ota_port_remove(OTA_TEMP_PATH);
        memset(&ctx, 0, sizeof(ctx));
        return STATUS_ERROR;
    }

    size_t written = ota_port_write(data, 1, len, ctx.fp);
    if (written != len) {
        log_error("OTA write failed: written=%u expected=%u error_count=%u",
                  (uint32_t)written, (uint32_t)len, ctx.error_count + 1);
        ctx.error_count++;
        if (ctx.error_count > 3) {
            ota_port_close(ctx.fp);
            ctx.fp = NULL;
            ota_port_remove(OTA_TEMP_PATH);
            memset(&ctx, 0, sizeof(ctx));
            return STATUS_ERROR;
        }
        return STATUS_ERROR;
    }

    ctx.received_size += len;
    ctx.crc_calculated = ota_crc32_update(ctx.crc_calculated, data, len);
    ctx.last_time = ota_port_time();

    if ((ctx.received_size == ctx.expected_size) ||
        ((ctx.received_size % (64 * 1024)) < len)) {
        log_info("OTA progress: %u/%u bytes",
                 ctx.received_size, ctx.expected_size);
    }

    return STATUS_OK;
}

STATUS_E ota_stop(void)
{
    /*
    * Syncs, closes the temporary file and validates the received data against the
    * expected size and CRC. If validation passes, it applies the update by
    * renaming the temporary file to the final path. Always resets context.
    */
   log_debug("OTA STOP: received=%u expected=%u crc_calc=0x%08X crc_exp=0x%08X",
          ctx.received_size,
          ctx.expected_size,
          ctx.crc_calculated ^ 0xFFFFFFFFu,
          ctx.crc_expected);
    
    if (ctx.fp == NULL) {
        log_error("OTA stop failed: no active OTA session");
        return STATUS_ERROR;
    }

    if (ota_port_sync(ctx.fp) != 0) {
        log_error("OTA sync failed: %s", ota_port_last_error());
        ota_port_close(ctx.fp);
        ctx.fp = NULL;
        ota_port_remove(OTA_TEMP_PATH);
        memset(&ctx, 0, sizeof(ctx));
        return STATUS_ERROR;
    }

    ota_port_close(ctx.fp);
    ctx.fp = NULL;

    if (ctx.received_size != ctx.expected_size) {
        log_error("OTA size mismatch: received=%u expected=%u",
                  ctx.received_size, ctx.expected_size);
        ota_port_remove(OTA_TEMP_PATH);
        memset(&ctx, 0, sizeof(ctx));
        return STATUS_ERROR;
    }

    uint32_t crc = ctx.crc_calculated ^ 0xFFFFFFFFu;
    if (crc != ctx.crc_expected) {
        log_error("OTA CRC mismatch: calculated=0x%08X expected=0x%08X",
                  crc, ctx.crc_expected);
        ota_port_remove(OTA_TEMP_PATH);
        memset(&ctx, 0, sizeof(ctx));
        return STATUS_ERROR;
    }

    log_info("OTA validation OK: size=%u crc=0x%08X",
             ctx.received_size, crc);

    STATUS_E ret = ota_apply_update();
    if (ret == STATUS_OK) {
        log_info("OTA successful, reboot scheduled in 2 seconds");
        //system("systemctl restart my_app.service"); //Use this once the buildroot is implemented
        ota_port_system("(sleep 2; reboot) &"); // For now just reboot the system to apply the update
    }
    else {
        log_error("Failed to apply OTA update");
    }
    memset(&ctx, 0, sizeof(ctx));
    return ret;
}

STATUS_E ota_check_timeout(int timeout_sec)
{
    if (ctx.fp == NULL) {
        return STATUS_OK;
    }

    if (ota_port_time() - ctx.last_time > timeout_sec) {
        log_error("OTA timeout: received=%u expected=%u",
                  ctx.received_size, ctx.expected_size);
        if (ctx.fp) {
            ota_port_close(ctx.fp);
            ctx.fp = NULL;
        }
        ota_port_remove(OTA_TEMP_PATH);
        memset(&ctx, 0, sizeof(ctx));
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

ota_ops_t ota_get_ops(void)
{
    return ops;
}
