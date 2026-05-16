/*
* name: ota_client.c
* Description: This file contains the implementation of the OTA (Over-The-Air)
* update functionality. It download a new binary and stores it in a temporary 
* location /opt/my_app/upfate/my_app.new and once it is validated it replaces
* the current binary /usr/local/bin/my_app with the new one. It is necessary to
* implement a second CRC32 check (in addition to the one in the protocol) to 
* ensure that the file is not corrupted during the update process.
*/
#include "ota_client.h"
#include "tcp_client.h"
#include "tcp_frame_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define CHUNK_SIZE 256
#define MAX_RETRIES 3

static uint32_t ota_crc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320u;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

static void ota_write_be32(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t)(value >> 24);
    out[1] = (uint8_t)(value >> 16);
    out[2] = (uint8_t)(value >> 8);
    out[3] = (uint8_t)(value & 0xFF);
}

static int load_firmware(const char *path, uint8_t **out, uint32_t *size, 
    uint32_t *crc, uint8_t *header)
{
    /*Load firmware from file and calculate its size and CRC32*/
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open %s\n", path);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    uint32_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *data = malloc(file_size);
    if (data == NULL) {
        fclose(fp);
        return -1;
    }
    fread(data, 1, file_size, fp);
    fclose(fp);

    *out = data;
    *size = file_size;
    *crc = ota_crc32(data, file_size);
    ota_write_be32(header, file_size);
    ota_write_be32(header + 4, *crc);
    return 0;
}

static int send_frame_with_ack(uint8_t *frame, int len)
{
    /*Send a frame and wait for ACK, retrying if necessary*/
    int retries = 0;
    char recv_buf[FRAME_MAX_BUFFER];
    int recv_len;
    frame_t recv_frame;
    int consumed;

    while (retries < MAX_RETRIES) {
        tcp_client_send((char*)frame, len);
        sleep(1);
        recv_len = tcp_client_recv(recv_buf, sizeof(recv_buf));
        if (recv_len > 0) {
            if (frame_decode((uint8_t*)recv_buf, recv_len, &recv_frame, 
                    &consumed) == 1) {
                if (recv_frame.cmd == CMD_ACK) { 
                    return 0; // ACK received
                } else if (recv_frame.cmd == CMD_NACK) {
                    retries++;
                }
            } else {
                retries++;
            }
        } else {
            retries++;
        }
    }
    return -1; // Failed after retries
}

int ota_client_send_update(const char *ip, const char *port, const char *path)
{
    /*
    * Send OTA update to the specified IP and port. Firmware is sent in chunks 
    * with a START frame containing the header (size and CRC) followed by DATA 
    * frames and an END frame. Each frame requires an ACK from the server, and
    * the client will retry sending a frame up to MAX_RETRIES times if it does 
    * not receive an ACK. In case ACK is not received after retries, the
    * function will return an error and the client will disconnect.
    */
    uint8_t *data;
    uint32_t size;
    uint32_t crc;
    uint8_t header[8];

    if (load_firmware(path, &data, &size, &crc, header) != 0) {
        return -1;
    }

    printf("Firmware: %s (%u bytes, crc=0x%08X)\n", path, size, crc);

    if (tcp_client_connect(ip, port) != 0) {
        free(data);
        return -1;
    }

    uint8_t buffer[FRAME_MAX_BUFFER];

    // Send START
    int len = frame_encode(CMD_OTA_START, header, 8, buffer);
    if (len < 0) {
        tcp_client_disconnect();
        free(data);
        return -1;
    }
    printf("Sent OTA_START\n");
    if (send_frame_with_ack(buffer, len) != 0) {
        tcp_client_disconnect();
        free(data);
        return -1;
    }

    // Send chunks 
    uint32_t sent = 0;
    int chunk_num = 1;
    int total_chunks = (size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    while (sent < size) {
        uint16_t chunk_len = (size - sent > CHUNK_SIZE) ? CHUNK_SIZE : size - sent;
        len = frame_encode(CMD_OTA_DATA, data + sent, chunk_len, buffer);
        if (len < 0) break;
        printf("Sent chunk %d/%d\n", chunk_num, total_chunks);
        if (send_frame_with_ack(buffer, len) != 0) break;
        sent += chunk_len;
        chunk_num++;
    }

    if (sent < size) {
        tcp_client_disconnect();
        free(data);
        return -1;
    }

    // Send END
    len = frame_encode(CMD_OTA_END, NULL, 0, buffer);
    if (len > 0) {
        printf("Sent OTA_END\n");
        if (send_frame_with_ack(buffer, len) != 0) {
            tcp_client_disconnect();
            free(data);
            return -1;
        }
    }

    tcp_client_disconnect();
    free(data);
    return 0;
}
