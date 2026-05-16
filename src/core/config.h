// Name: config.h
// This file contains configuration settings for the application. 
// It is included in the main.c file and can be used to define constants, 
// macros, and other configuration parameters that are needed throughout 
// the application.

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define ENABLE_GPIO     0
#define ENABLE_I2C      0
#define ENABLE_NETWORK  0
#define ENABLE_LOGGING  1

// Status codes
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR,
    STATUS_INVALID_PARAM,
    STATUS_INCOMPLETE,
    STATUS_TIMEOUT
} STATUS_E;

// Macros for common operations
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Macros for error handle
# define PARAM_CHECK_MAX(a, max) do { \
    if ((a) > (max)) { \
        return STATUS_INVALID_PARAM; \
    } \
} while(0)

#define PARAM_CHECK_MIN(a, min) do { \
    if ((a) < (min)) { \
        return STATUS_INVALID_PARAM; \
    } \
} while(0)

#define PARAM_CHECK_RANGE(a, min, max) do { \
    if ((a) < (min) || (a) > (max)) { \
        return STATUS_INVALID_PARAM; \
    } \
} while(0)

#define PARAM_CHECK_MAX_EQUAL(a, max) do { \
    if ((a) >= (max)) { \
        return STATUS_INVALID_PARAM; \
    } \
} while(0)

#define PARAM_CHECK_MIN_EQUAL(a, min) do { \
    if ((a) <= (min)) { \
        return STATUS_INVALID_PARAM; \
    } \
} while(0)

#define PARAM_CHECK_EQUAL(a, val) do { \
    if ((a) != (val)) { \
        return STATUS_INVALID_PARAM; \
    } \
} while(0)

#endif // APP_CONFIG_H
