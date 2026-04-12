// name: logger.h
// Description: This file contains the declarations for the logging
// functionality of the application. It is included in the main.c file

#ifndef LOGGER_H
#define LOGGER_H

#include "app/config.h"

#ifdef ENABLE_LOGGING

void log_info(const char *message);
void log_error(const char *message);
void log_debug(const char *message);

#else
// If logging is disabled, define empty functions to avoid compilation errors
#define log_info(message)  ((void)0)
#define log_error(message) ((void)0)
#define log_debug(message) ((void)0)

#endif // ENABLE_LOGGING

#endif // LOGGER_H