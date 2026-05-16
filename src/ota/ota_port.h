/*
* name: ota_port.h
* Description: This file contains the system abstraction used by the OTA
* module. It keeps the OTA logic independent from POSIX calls so it can be
* mocked in unit tests.
*/

#ifndef OTA_PORT_H
#define OTA_PORT_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef struct ota_port_file ota_port_file_t;

const char *ota_port_last_error(void);
int ota_port_error_is_not_found(void);

int ota_port_mkdir_parent(const char *path);
ota_port_file_t *ota_port_open(const char *path, const char *mode);
size_t ota_port_write(const void *data, size_t size, size_t count, ota_port_file_t *file);
int ota_port_sync(ota_port_file_t *file);
int ota_port_close(ota_port_file_t *file);

int ota_port_remove(const char *path);
int ota_port_rename(const char *old_path, const char *new_path);
int ota_port_chmod(const char *path, uint32_t mode);

time_t ota_port_time(void);
int ota_port_system(const char *command);

#endif /* OTA_PORT_H */
