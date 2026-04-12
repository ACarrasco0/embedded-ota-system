// name: logger.c
// Description: This file contains the implementation of the logging 
// functionality for the application.

#include "app/config.h"
#include <stdio.h>
#include "modules/logger/logger.h" 
 
#ifdef ENABLE_LOGGING

void log_info(const char *message){
    printf("[INFO][PID:%d] %s\n", getpid(), message);
}

void log_error(const char *message){
    printf("[ERROR][PID:%d] %s\n", getpid(), message);
}

void log_debug(const char *message){
    printf("[DEBUG][PID:%d] %s\n", getpid(), message);
}

#endif // ENABLE_LOGGING