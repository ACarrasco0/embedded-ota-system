#include "platform/signals.h"
#include "logger/logger.h"
#include "core/scheduler.h"
#include <signal.h>
#include <stdint.h>


volatile uint8_t app_running = 1;  

/**
* @brief Handle the signal (e.g., SIGINT, SIGTERM) and set the app_running flag to 0
* @param sig (int) The signal number (unused)
* @param info (siginfo_t *) Pointer to siginfo_t structure containing signal information (unused)
* @param ucontext (void *) Pointer to ucontext_t structure containing context information (unused)
* @return void
*/
static void handle_signal(int sig, siginfo_t *info, void *ucontext) {
    // Handle the signal (e.g., SIGINT, SIGTERM) and set the app_running flag to 0
    (void)sig;          // Unused parameter
    (void)info;         // Unused parameter
    (void)ucontext;     // Unused parameter
    log_info("Signal received, shutting down...");
    app_running = 0;
    tcp_server_task_running = 0;
}

/**
* @brief Set up signal handlers for SIGINT and SIGTERM
* @return void
*/
void setup_signals(void) {
    struct sigaction sa;
    sa.sa_sigaction = handle_signal;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);   // CTRL+C
    sigaction(SIGTERM, &sa, NULL);  // Termination signal
}