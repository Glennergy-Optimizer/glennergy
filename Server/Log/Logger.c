/**
 * @file Logger.c
 * @brief Implementation of asynchronous logging using pipe and child process.
 * @ingroup Logger
 */

#include "Logger.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG_MSG_MAX 256

/**
 * @struct LogMessage
 * @brief Message payload written by the logger.
 *
 * @note All fields are copied before output.
 * @note Fixed-size buffers may truncate long values.
 */
typedef struct {
    time_t timestamp;          /**< Timestamp of log entry */
    pid_t pid;                 /**< Process ID */
    char level[16];            /**< Log level string */
    char module[32];           /**< Module name */
    char message[LOG_MSG_MAX]; /**< Log message content */
} LogMessage;

static LogLevel log_level = LOG_LEVEL_INFO;
static bool logger_initialized = false;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Internal functions */

/**
 * @brief Returns the syslog-style priority for a log level.
 *
 * @param level Log level.
 * @return Priority value used when formatting the output line.
 */
static int log_JournalPriority(LogLevel level);

/**
 * @brief Writes a log message to the selected output stream.
 *
 * @param log_msg Pointer to log message.
 * @param level Log level.
 *
 * @note Writes a single formatted line and flushes the stream.
 */
static void log_ToJournal(const LogMessage* log_msg, LogLevel level);

/**
 * @brief Returns the string representation of a log level.
 *
 * @param level Log level.
 * @return String representation of the level.
 */
const char* log_GetLevelString(LogLevel level);

/**
 * @brief Initializes logger system.
 */
int log_Init(const char* filename)
{
    (void)filename;

    pthread_mutex_lock(&log_mutex);
    logger_initialized = true;
    pthread_mutex_unlock(&log_mutex);

    return 0;
}

/**
 * @brief Logs a message.
 */
void log_Message(LogLevel level, const char* module, const char* msg)
{
    if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_ERROR || msg == NULL)
        return;

    pthread_mutex_lock(&log_mutex);

    if (!logger_initialized || level < log_level)
    {
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    LogMessage log_msg = {0};
    log_msg.timestamp = time(NULL);
    log_msg.pid = getpid();
    snprintf(log_msg.level, sizeof(log_msg.level), "%s", log_GetLevelString(level));
    snprintf(log_msg.module, sizeof(log_msg.module), "%s", module != NULL ? module : "UNKNOWN");
    snprintf(log_msg.message, sizeof(log_msg.message), "%s", msg);

    size_t message_length = strlen(log_msg.message);
    while (message_length > 0 &&
           (log_msg.message[message_length - 1] == '\n' || log_msg.message[message_length - 1] == '\r'))
    {
        log_msg.message[--message_length] = '\0';
    }

    log_ToJournal(&log_msg, level);
    pthread_mutex_unlock(&log_mutex);
}

/**
 * @brief Logs a formatted message.
 */
void log_MessageFmt(LogLevel level, const char* module, const char* fmt, ...)
{
    if (fmt == NULL)
        return;

    char buffer[LOG_MSG_MAX];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    log_Message(level, module, buffer);
}

static int log_JournalPriority(LogLevel level)
{
    switch (level)
    {
        case LOG_LEVEL_DEBUG: return 7;
        case LOG_LEVEL_INFO:  return 6;
        case LOG_LEVEL_WARN:  return 4;
        case LOG_LEVEL_ERROR: return 3;
        default:              return 6;
    }
}

static void log_ToJournal(const LogMessage* log_msg, LogLevel level)
{
    FILE* stream = level >= LOG_LEVEL_WARN ? stderr : stdout;

    fprintf(stream, "<%d>[%s] [%s] %s\n",
            log_JournalPriority(level),
            log_msg->level,
            log_msg->module,
            log_msg->message);
    fflush(stream);
}

const char* log_GetLevelString(LogLevel level)
{
    switch(level)
    {
        case LOG_LEVEL_DEBUG:   return "DEBUG";
        case LOG_LEVEL_INFO:    return "INFO";
        case LOG_LEVEL_WARN:    return "WARNING";
        case LOG_LEVEL_ERROR:   return "ERROR";
        default: return "INFO";
    }
}

void log_SetLevel(LogLevel level)
{
    if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_ERROR)
        return;

    pthread_mutex_lock(&log_mutex);
    log_level = level;
    pthread_mutex_unlock(&log_mutex);
}

void log_CloseWrite(void)
{
    pthread_mutex_lock(&log_mutex);
    fflush(stdout);
    fflush(stderr);
    pthread_mutex_unlock(&log_mutex);
}

void log_Cleanup(void)
{
    pthread_mutex_lock(&log_mutex);
    fflush(stdout);
    fflush(stderr);
    logger_initialized = false;
    pthread_mutex_unlock(&log_mutex);
}
