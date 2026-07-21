/**
 * @file SignalHandler.c
 * @brief Implementation of signal handling utilities.
 *
 * @ingroup SignalHandler
 */

#include <signal.h>
#include <stddef.h>
#include <sys/types.h>
#include "SignalHandler.h"

/**
 * @brief Flag indicating whether a termination signal has been received.
 */
static sig_atomic_t stop = 0;

/**
 * @brief Signal handler function.
 *
 * Sets the stop flag when a signal is received.
 *
 * @param sig The signal number received.
 */
void SignalHandler_Handle(int sig)
{
    (void)sig;
    stop = 1;
}

/**
 * @brief Implementation of SignalHandler_Initialize.
 *
 * See header for full contract documentation.
 */
void SignalHandler_Initialize()
{
    struct sigaction action = {0};
    action.sa_handler = SignalHandler_Handle;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    // Suggestion: Consider using sigaction() instead of signal() for more robust and portable signal handling.
}

/**
 * @brief Returns the current stop flag state.
 *
 * @return int Non-zero if a termination signal has been received, otherwise 0.
 */
int SignalHandler_Stop()
{
    return stop;

    // Suggestion: Could be marked as inline in header if used frequently and performance-critical.
}
