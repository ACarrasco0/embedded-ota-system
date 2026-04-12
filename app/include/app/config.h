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
#define STATUS_OK            0
#define STATUS_ERROR         1
#define STATUS_INVALID_PARAM 2
#define STATUS_TIMEOUT       3

// Macros for common operations
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Macros for error handle

#endif // APP_CONFIG_H
