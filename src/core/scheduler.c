#include "scheduler.h"
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include "config.h"
#include "logger/logger.h"
#include "comms/tcp_server_task.h"
#include "comms/tcp_server.h"

volatile uint8_t tcp_server_task_running = 0;

uint8_t dummy_count = 0;
uint8_t dummy_count2 = 0;

void system_dummy_task(void)
{
    printf("[SCHEDULER] Hello from 1: %d \n", dummy_count);
    dummy_count++;
}

void system_dummy_task2(void)
{
    printf("[SCHEDULER] Hello from 2: %d \n", dummy_count2);
    dummy_count2++;
}

static task_t tasks[] = {
    //{ system_dummy_task,  3000, 0 }, 
    //{ system_dummy_task2, 4000, 0 },
    { tcp_server_task, 200, 0 } 
}; 

#define TASK_COUNT (sizeof(tasks) / sizeof(tasks[0]))

unsigned long scheduler_get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000UL) + (tv.tv_usec / 1000UL);
}

void scheduler_init(void)
{
    printf("[SCHEDULER] Initialized with %zu tasks\n", TASK_COUNT);
    printf("[SCHEDULER] I am updated :)\n");
    STATUS_E status = tcp_server_init();
    if (status != STATUS_OK) {
        log_error("Failed to initialize TCP server");
    } else {
        tcp_server_task_running = 1;
    }
}

void scheduler_deinit(void)
{
    printf("[SCHEDULER] Deinitializing\n");

    STATUS_E status = tcp_server_deinit();
    if (status != STATUS_OK) {
        log_error("Failed to deinitialize TCP server");
    } 
}
void scheduler_run(void)
{
    unsigned long now = scheduler_get_time_ms();

    for (unsigned int i = 0; i < TASK_COUNT; i++) {

        if (tasks[i].task_func == NULL) {
            continue;
        }

        if ((now - tasks[i].last_run_ms) >= tasks[i].period_ms) {
            tasks[i].task_func();
            tasks[i].last_run_ms = now;
        }
    }
}