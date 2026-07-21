#ifndef LOGGER_H
#define LOGGER_H
#include <time.h>

/**
 * @file Logger.h
 * @brief Logging interface for levels, formatting, and asynchronous output.
 *
 * @defgroup Logger Logger
 * @brief Logger interface and log-level helpers.
 * @{
 */

/**
 * @enum LogLevel
 * @brief Log levels used by the logger.
 */
typedef enum {
    LOG_LEVEL_DEBUG,  /**< Debug information. */
    LOG_LEVEL_INFO,   /**< Standard informational messages. */
    LOG_LEVEL_WARN,   /**< Warnings that may require attention. */
    LOG_LEVEL_ERROR   /**< Errors that must be addressed. */
} LogLevel;

/**
 * @def MODULE_NAME
 * @brief Name of the module being logged.
 *
 * Must be defined in each .c file before including Logger.
 *
 * @note Defaults to "UNKNOWN" if not explicitly defined.
 */
#ifndef MODULE_NAME
#define MODULE_NAME "UNKNOWN"
#endif

/**
 * @def LOG_DEBUG
 * @brief Logs a debug message with the module name.
 */
#define LOG_DEBUG(fmt, ...)   log_MessageFmt(LOG_LEVEL_DEBUG, MODULE_NAME, fmt, ##__VA_ARGS__)

/**
 * @def LOG_INFO
 * @brief Logs an info message with the module name.
 */
#define LOG_INFO(fmt, ...)    log_MessageFmt(LOG_LEVEL_INFO, MODULE_NAME, fmt, ##__VA_ARGS__)

/**
 * @def LOG_WARNING
 * @brief Logs a warning message with the module name.
 */
#define LOG_WARNING(fmt, ...) log_MessageFmt(LOG_LEVEL_WARN, MODULE_NAME, fmt, ##__VA_ARGS__)

/**
 * @def LOG_ERROR
 * @brief Logs an error message with the module name.
 */
#define LOG_ERROR(fmt, ...)   log_MessageFmt(LOG_LEVEL_ERROR, MODULE_NAME, fmt, ##__VA_ARGS__)

/**
 * @brief Initializes the logger system.
 *
 * @param log_path Path to the log file. If NULL, a default filename is used.
 *
 * @return 0 on success, -1 on failure.
 *
 * @note Creates the logging pipe and starts the logging process.
 * @warning Uses fork() and pipe(), creating a separate process.
 */
int log_Init(const char* log_path);

/**
 * @brief Logs a message directly.
 *
 * @param level Log level.
 * @param module Module name.
 * @param msg Message string to log.
 *
 * @warning Thread-safe via an internal mutex.
 */
void log_Message(LogLevel level, const char* module, const char* msg);

/**
 * @brief Logs a formatted message.
 *
 * @param level Log level.
 * @param module Module name.
 * @param fmt printf-style format string.
 *
 * @note Internally formats the string before forwarding to log_Message().
 */
void log_MessageFmt(LogLevel level, const char* module, const char* fmt, ...);

/**
 * @brief Cleans up logger resources.
 *
 * @warning Blocks until the logger process exits.
 */
void log_Cleanup(void);

/**
 * @brief Sets the global log level.
 *
 * @param level New log level.
 *
 * @note Messages below this level are ignored.
 */
void log_SetLevel(LogLevel level);

/**
 * @brief Returns the string representation of a log level.
 *
 * @param level Log level.
 *
 * @return String: "DEBUG", "INFO", "WARNING", or "ERROR".
 */
const char* log_GetLevelString(LogLevel level);

/**
 * @brief Closes the write file descriptor.
 *
 * @note Useful when shutting down the parent while allowing the logger to flush.
 */
void log_CloseWrite(void);

/** @} */

#endif // LOGGER_H
