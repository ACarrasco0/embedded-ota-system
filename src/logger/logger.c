// name: logger.c
// Description: This file contains the implementation of the logging 
// functionality for the application. Totally unnecesary but looked cool.

#include "core/config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include "logger.h"

#ifdef ENABLE_LOGGING
 
static void log_base(const char *level, const char *fmt, va_list args) {
    printf("[%s][PID:%d] ", level, getpid());
    vprintf(fmt, args);
    printf("\n");
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_base("INFO", fmt, args);
    va_end(args);
}
    
void log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_base("ERROR", fmt, args);
    va_end(args);
}

void log_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_base("DEBUG", fmt, args);
    va_end(args);
}

#endif // ENABLE_LOGGING