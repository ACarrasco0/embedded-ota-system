#include <unistd.h>
#include "core/config.h"
#include "logger/logger.h"
#include "platform/system.h"
#include "platform/signals.h"
#include "core/scheduler.h"

int main(void)
{
    setup_signals();

    system_init();
    scheduler_init();

    log_info("App started");

    while (app_running) {

        scheduler_run();

        usleep(10000);
    }

    system_deinit();
    scheduler_deinit();

    log_info("App stopped");

    return 0;
}