/*
* name: ota_port_posix.c
* Description: This file contains the POSIX implementation of the OTA system
* abstraction. Filesystem and OS calls used by the OTA module are kept here.
*/

#include "ota/ota_port.h"
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct ota_port_file {
    FILE *fp;
};

const char *ota_port_last_error(void)
{
    return strerror(errno);
}

int ota_port_error_is_not_found(void)
{
    return errno == ENOENT;
}

static int mkdir_p(const char *path)
{
    char tmp[256];
    size_t len;

    if (!path || !*path) {
        return -1;
    }

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);

    if (len == 0) {
        return -1;
    }

    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
    }

    return 0;
}

int ota_port_mkdir_parent(const char *path)
{
    char *path_copy;
    char *dir;
    int ret;

    path_copy = strdup(path);
    if (path_copy == NULL) {
        return -1;
    }

    dir = dirname(path_copy);
    ret = mkdir_p(dir);

    free(path_copy);
    return ret;
}

ota_port_file_t *ota_port_open(const char *path, const char *mode)
{
    ota_port_file_t *file;

    file = malloc(sizeof(*file));
    if (file == NULL) {
        return NULL;
    }

    file->fp = fopen(path, mode);
    if (file->fp == NULL) {
        free(file);
        return NULL;
    }

    return file;
}

size_t ota_port_write(const void *data, size_t size, size_t count, ota_port_file_t *file)
{
    if (file == NULL || file->fp == NULL) {
        return 0;
    }

    return fwrite(data, size, count, file->fp);
}

int ota_port_sync(ota_port_file_t *file)
{
    if (file == NULL || file->fp == NULL) {
        return -1;
    }

    return fsync(fileno(file->fp));
}

int ota_port_close(ota_port_file_t *file)
{
    int ret;

    if (file == NULL || file->fp == NULL) {
        free(file);
        return -1;
    }

    ret = fclose(file->fp);
    free(file);
    return ret;
}

int ota_port_remove(const char *path)
{
    return remove(path);
}

int ota_port_rename(const char *old_path, const char *new_path)
{
    return rename(old_path, new_path);
}

int ota_port_chmod(const char *path, uint32_t mode)
{
    return chmod(path, mode);
}

time_t ota_port_time(void)
{
    return time(NULL);
}

int ota_port_system(const char *command)
{
    return system(command);
}
