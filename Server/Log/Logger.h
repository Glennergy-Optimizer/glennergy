#ifndef LOGGER_H
#define LOGGER_H
#include <time.h>

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} LogLevel;

// Define MODULE_NAME in each .c file before including this header
#ifndef MODULE_NAME
#define MODULE_NAME "UNKNOWN"
#endif

#define LOG_DEBUG(fmt, ...)   log_MessageFmt(LOG_LEVEL_DEBUG, MODULE_NAME, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    log_MessageFmt(LOG_LEVEL_INFO, MODULE_NAME, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) log_MessageFmt(LOG_LEVEL_WARN, MODULE_NAME, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   log_MessageFmt(LOG_LEVEL_ERROR, MODULE_NAME, fmt, ##__VA_ARGS__)


int log_Init(const char* log_path);  // Pass NULL for default: Logs/log.txt
void log_Message(LogLevel level, const char* module, const char* msg);
void log_MessageFmt(LogLevel level, const char* module, const char* fmt, ...);
void log_Cleanup(void);

void log_SetLevel(LogLevel level);
const char* log_GetLevelString(LogLevel level);
void log_CloseWrite(void);

#endif // LOGGER_H