#ifndef SIGNALHANDLER_H 
#define SIGNALHANDLER_H

/**
 * @file SignalHandler.h
 * @brief Public API for signal handling utilities.
 *
 * Provides initialization and stop-state queries for process signal handling.
 */

/**
 * @defgroup SignalHandler SignalHandler
 * @brief Signal handling utilities.
 *
 * Initializes the process signal handlers used by the application and exposes
 * a flag that indicates whether termination was requested.
 * @{
 */

/**
 * @brief Initializes signal handlers for the application.
 */
void SignalHandler_Initialize();

/**
 * @brief Checks whether a stop signal has been received.
 *
 * @return Non-zero if a stop signal was received, otherwise 0.
 */
int SignalHandler_Stop();

/** @} */

#endif
