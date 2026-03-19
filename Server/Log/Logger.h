#ifndef LOGGER_H
#define LOGGER_H
#include <time.h>

/**
 * @enum LogLevel
 * @brief Definierar loggnivåer som används i loggern.
 */
typedef enum {
    LOG_LEVEL_DEBUG,  /**< Debug-information, mest detaljerad */
    LOG_LEVEL_INFO,   /**< Standardinformation */
    LOG_LEVEL_WARN,   /**< Varningar som kan kräva uppmärksamhet */
    LOG_LEVEL_ERROR   /**< Fel som måste åtgärdas */
} LogLevel;

/**
 * @def MODULE_NAME
 * @brief Namn på modulen som loggas. Definieras i varje .c-fil innan Logger inkluderas.
 */
#ifndef MODULE_NAME
#define MODULE_NAME "UNKNOWN"
#endif

/**
 * @def LOG_DEBUG(fmt, ...)
 * @brief Logga en debug-meddelande med modulnamn.
 */
#define LOG_DEBUG(fmt, ...)   log_MessageFmt(LOG_LEVEL_DEBUG, MODULE_NAME, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    log_MessageFmt(LOG_LEVEL_INFO, MODULE_NAME, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) log_MessageFmt(LOG_LEVEL_WARN, MODULE_NAME, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   log_MessageFmt(LOG_LEVEL_ERROR, MODULE_NAME, fmt, ##__VA_ARGS__)

/**
 * @brief Initierar loggern.
 * @param log_path Filväg för loggfilen. Passera NULL för default.
 * @return 0 vid framgång, -1 vid fel.
 */
int log_Init(const char* log_path);

/**
 * @brief Loggar ett meddelande direkt.
 * @param level Loggnivå.
 * @param module Modulnamn.
 * @param msg Meddelande som ska loggas.
 */
void log_Message(LogLevel level, const char* module, const char* msg);

/**
 * @brief Loggar ett formaterat meddelande.
 * @param level Loggnivå.
 * @param module Modulnamn.
 * @param fmt printf-liknande formatsträng.
 */
void log_MessageFmt(LogLevel level, const char* module, const char* fmt, ...);

/**
 * @brief Rensar loggerresurser och väntar på child-process.
 */
void log_Cleanup(void);

/**
 * @brief Sätter global loggnivå.
 * @param level Ny loggnivå.
 */
void log_SetLevel(LogLevel level);

/**
 * @brief Returnerar strängrepresentation av loggnivå.
 * @param level Loggnivå.
 * @return Sträng: "DEBUG", "INFO", "WARNING", "ERROR".
 */
const char* log_GetLevelString(LogLevel level);

/**
 * @brief Stänger skriv-FD utan att påverka child-process.
 */
void log_CloseWrite(void);

#endif // LOGGER_H