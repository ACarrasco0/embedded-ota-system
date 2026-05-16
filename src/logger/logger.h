// name: logger.h
// Description: This file contains the declarations for the logging
// functionality of the application. It is included in the main.c file

#ifndef LOGGER_H
#define LOGGER_H

#include "core/config.h"

#ifdef ENABLE_LOGGING

void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_debug(const char *fmt, ...);

#else
// If logging is disabled, define empty functions to avoid compilation errors
#define log_info(fmt, ...)  ((void)0)
#define log_error(fmt, ...) ((void)0)
#define log_debug(fmt, ...) ((void)0)

#endif // ENABLE_LOGGING

#endif // LOGGER_H