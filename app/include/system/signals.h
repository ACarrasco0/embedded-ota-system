// name: signals.h
// Description: This file contains the declarations for signal handling in the
// application.

#ifndef SIGNALS_H
#define SIGNALS_H

#include <stdint.h>

void setup_signals(void);
extern volatile uint8_t app_running;  

#endif // SIGNALS_H