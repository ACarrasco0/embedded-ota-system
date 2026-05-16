#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>
#include <stdint.h>

typedef void (*task_func_t)(void);

extern volatile uint8_t tcp_server_task_running;  

typedef struct {
    task_func_t task_func;
    unsigned int period_ms;
    unsigned long last_run_ms;
} task_t;


void scheduler_init(void);
void scheduler_run(void);
void scheduler_deinit(void);

unsigned long scheduler_get_time_ms(void);

#endif // SCHEDULER_H