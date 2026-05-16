/*
* name: ota_port_mock.c
* Description: This file contains a mock implementation of the OTA port layer.
* It is used by unit tests to validate OTA logic without touching the real
* filesystem or operating system.
*/

#include "ota/ota_port.h"
#include <string.h>

#define OTA_MOCK_TEMP_PATH "/opt/my_app/update/my_app.new"
#define OTA_MOCK_FINAL_PATH "/opt/my_app/current/my_app"
#define OTA_MOCK_BACKUP_PATH "/opt/my_app/backup/my_app.bak"
#define OTA_MOCK_MAX_FILE_SIZE (10 * 1024 * 1024)

struct ota_port_file {
    int open;
};

static struct ota_port_file mock_file;
static uint8_t mock_file_data[OTA_MOCK_MAX_FILE_SIZE];
static size_t mock_file_size;
static time_t mock_time;
static const char *mock_last_error;
static int mock_not_found;

static int mock_mkdir_parent_count;
static int mock_open_count;
static int mock_write_count;
static int mock_sync_count;
static int mock_close_count;
static int mock_remove_count;
static int mock_rename_count;
static int mock_chmod_count;
static int mock_system_count;

static int mock_fail_mkdir_parent;
static int mock_fail_open;
static int mock_fail_sync;
static int mock_fail_chmod;
static size_t mock_write_limit;

void ota_port_mock_reset(void)
{
    memset(mock_file_data, 0, sizeof(mock_file_data));
    memset(&mock_file, 0, sizeof(mock_file));
    mock_file_size = 0;
    mock_time = 1000;
    mock_last_error = "mock error";
    mock_not_found = 0;

    mock_mkdir_parent_count = 0;
    mock_open_count = 0;
    mock_write_count = 0;
    mock_sync_count = 0;
    mock_close_count = 0;
    mock_remove_count = 0;
    mock_rename_count = 0;
    mock_chmod_count = 0;
    mock_system_count = 0;

    mock_fail_mkdir_parent = 0;
    mock_fail_open = 0;
    mock_fail_sync = 0;
    mock_fail_chmod = 0;
    mock_write_limit = OTA_MOCK_MAX_FILE_SIZE;
}

void ota_port_mock_advance_time(time_t seconds)
{
    mock_time += seconds;
}

void ota_port_mock_set_fail_mkdir_parent(int fail)
{
    mock_fail_mkdir_parent = fail;
}

void ota_port_mock_set_fail_open(int fail)
{
    mock_fail_open = fail;
}

void ota_port_mock_set_fail_sync(int fail)
{
    mock_fail_sync = fail;
}

void ota_port_mock_set_fail_chmod(int fail)
{
    mock_fail_chmod = fail;
}

void ota_port_mock_set_write_limit(size_t limit)
{
    mock_write_limit = limit;
}

size_t ota_port_mock_file_size(void)
{
    return mock_file_size;
}

int ota_port_mock_mkdir_parent_count(void)
{
    return mock_mkdir_parent_count;
}

int ota_port_mock_open_count(void)
{
    return mock_open_count;
}

int ota_port_mock_sync_count(void)
{
    return mock_sync_count;
}

int ota_port_mock_close_count(void)
{
    return mock_close_count;
}

int ota_port_mock_remove_count(void)
{
    return mock_remove_count;
}

int ota_port_mock_rename_count(void)
{
    return mock_rename_count;
}

int ota_port_mock_chmod_count(void)
{
    return mock_chmod_count;
}

int ota_port_mock_system_count(void)
{
    return mock_system_count;
}

const char *ota_port_last_error(void)
{
    return mock_last_error;
}

int ota_port_error_is_not_found(void)
{
    return mock_not_found;
}

int ota_port_mkdir_parent(const char *path)
{
    (void)path;
    mock_mkdir_parent_count++;
    mock_not_found = 0;

    if (mock_fail_mkdir_parent) {
        return -1;
    }

    return 0;
}

ota_port_file_t *ota_port_open(const char *path, const char *mode)
{
    mock_open_count++;
    mock_not_found = 0;

    if (strcmp(path, OTA_MOCK_TEMP_PATH) != 0 || strcmp(mode, "wb") != 0) {
        return NULL;
    }

    if (mock_fail_open) {
        return NULL;
    }

    mock_file.open = 1;
    mock_file_size = 0;
    return &mock_file;
}

size_t ota_port_write(const void *data, size_t size, size_t count, ota_port_file_t *file)
{
    size_t bytes = size * count;
    size_t available;

    mock_write_count++;
    mock_not_found = 0;

    if (file == NULL || !file->open || mock_file_size >= mock_write_limit) {
        return 0;
    }

    available = mock_write_limit - mock_file_size;
    if (bytes > available) {
        bytes = available;
    }

    if (mock_file_size + bytes > sizeof(mock_file_data)) {
        bytes = sizeof(mock_file_data) - mock_file_size;
    }

    memcpy(mock_file_data + mock_file_size, data, bytes);
    mock_file_size += bytes;

    return bytes / size;
}

int ota_port_sync(ota_port_file_t *file)
{
    mock_sync_count++;
    mock_not_found = 0;

    if (file == NULL || !file->open || mock_fail_sync) {
        return -1;
    }

    return 0;
}

int ota_port_close(ota_port_file_t *file)
{
    mock_close_count++;
    mock_not_found = 0;

    if (file == NULL || !file->open) {
        return -1;
    }

    file->open = 0;
    return 0;
}

int ota_port_remove(const char *path)
{
    (void)path;
    mock_remove_count++;
    mock_not_found = 0;
    return 0;
}

int ota_port_rename(const char *old_path, const char *new_path)
{
    mock_rename_count++;

    if (strcmp(old_path, OTA_MOCK_FINAL_PATH) == 0 &&
        strcmp(new_path, OTA_MOCK_BACKUP_PATH) == 0) {
        mock_not_found = 1;
        return -1;
    }

    if (strcmp(old_path, OTA_MOCK_TEMP_PATH) == 0 &&
        strcmp(new_path, OTA_MOCK_FINAL_PATH) == 0) {
        mock_not_found = 0;
        return 0;
    }

    mock_not_found = 0;
    return 0;
}

int ota_port_chmod(const char *path, uint32_t mode)
{
    mock_chmod_count++;
    mock_not_found = 0;

    if (strcmp(path, OTA_MOCK_FINAL_PATH) != 0 || mode != 0755 || mock_fail_chmod) {
        return -1;
    }

    return 0;
}

time_t ota_port_time(void)
{
    return mock_time;
}

int ota_port_system(const char *command)
{
    (void)command;
    mock_system_count++;
    return 0;
}
