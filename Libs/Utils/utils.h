#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @brief Utility functions for time handling and filesystem operations.
 * @defgroup utils Utility Functions
 * @{
 */

// System makron som denna ska komma före headers
// Vi säter alltid posic manuellt med GC Flags vid kompilering
//#define _POSIX_C_SOURCE 200809L 

// Vanliga headers
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <errno.h> // Den här för create_Folder, resten för system monotonic grejer
#include <stdio.h>

// Plattformsepcifika headers 
#if defined(_WIN32)
    #include <windows.h>
#else
    #include <sys/stat.h>
#endif

// ==============================
// Time utilities
// ==============================

/**
 * @brief Returns monotonic system time in milliseconds.
 *
 * @return Monotonic time in milliseconds.
 *
 * @note Uses CLOCK_MONOTONIC.
 * @note Not affected by system clock changes.
 *
 * @warning Requires POSIX support for clock_gettime().
 */
static inline uint64_t SystemMonotonicMS()
{
    long ms;
    time_t s;

    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    s = spec.tv_sec;
    ms = (spec.tv_nsec / 1000000);

    uint64_t result = s;
    result *= 1000;
    result += ms;

    return result;
}

/**
 * @brief Writes today's date into buffer (format: YYYY/MM-DD).
 *
 * @param buffer Output buffer.
 * @param size Size of buffer.
 *
 * @pre `buffer` must be valid and large enough.
 *
 * @note Uses localtime().
 */
static inline void GetTodayDate(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d/%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

/**
 * @brief Writes tomorrow's date into buffer (format: YYYY/MM-DD).
 *
 * @param buffer Output buffer.
 * @param size Size of buffer.
 *
 * @pre `buffer` must be valid and large enough.
 *
 * @warning Does not handle month/year overflow correctly.
 * @note Uses localtime().
 */
static inline void GetTomorrowDate(char *buffer, size_t size) {
    time_t t = time(NULL);
    t += 24 * 60 * 60;
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d/%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

/**
 * @brief Writes today's date formatted for filenames (YYYY-MM-DD).
 *
 * @param buffer Output buffer.
 * @param size Size of buffer.
 *
 * @pre `buffer` must be valid.
 */
static inline void GetTodayDateFile(char *buffer, size_t size)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d-%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

/**
 * @brief Writes current time (HH:MM:SS) into buffer.
 *
 * @param buffer Output buffer.
 * @param size Size of buffer.
 *
 * @pre `buffer` must be valid.
 */
static inline void GetCurrentTime(char *buffer, size_t size)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%02d:%02d:%02d",
             tm.tm_hour, tm.tm_min, tm.tm_sec);
}

// ==============================
// Filesystem utilities
// ==============================

/**
 * @enum dir_result_t
 * @brief Result codes for directory creation.
 */
typedef enum
{
    DIR_CREATED = 0,         /**< Directory successfully created */
    DIR_ALREADY_EXISTS = 1,  /**< Directory already exists */
    DIR_ERROR = -1           /**< Error occurred */
} dir_result_t;

/**
 * @brief Creates a directory.
 *
 * @param path Path to directory.
 * @return dir_result_t indicating result.
 *
 * @pre `path` must not be NULL or empty.
 *
 * @note Cross-platform implementation (Windows/POSIX).
 */
static inline dir_result_t create_folder(const char *path)
{
    if (!path || *path == '\0')
        return DIR_ERROR;

    #if defined(_WIN32)
        BOOL success = CreateDirectoryA(path, NULL);
        if (!success)
        {
            DWORD err = GetLastError();
            if (err == ERROR_ALREADY_EXISTS)
                return DIR_ALREADY_EXISTS;
            return DIR_ERROR;
        }
    #else
        if (mkdir(path, 0777) != 0)
        {
            if (errno == EEXIST)
                return DIR_ALREADY_EXISTS;
            return DIR_ERROR;
        }
    #endif

    return DIR_CREATED;
}

/**
 * @brief Checks if a file has been modified.
 *
 * @param _FilePath Path to file.
 * @param _LastModified Pointer to stored last modified timestamp.
 *
 * @return
 * - 0 if no change
 * - 1 if file changed
 * - -1 if file does not exist or error
 *
 * @pre `_FilePath` must be valid.
 * @pre `_LastModified` must be valid pointer.
 *
 * @post `_LastModified` updated if change detected.
 *
 * @warning Uses stat(); platform-dependent behavior.
 * @note Special behavior: if *_LastModified == -1, initializes value.
 */
static inline int file_lastModified(const char *_FilePath, time_t* _LastModified)
{
    struct stat st;

    if (stat(_FilePath, &st) != 0)
    {
        LOG_ERROR("Property file doesnt exist\n");
        return -1;
    }

    // När vi skapar last_modified sätter vi den till -1, då vet vi att vi ska synka/hämta tiderna.
    if (*_LastModified == -1)
    {
        *_LastModified = st.st_mtime;
        return 0; 
    }

    if (st.st_mtime != *_LastModified)
    {
        *_LastModified = st.st_mtime;
        return 1;
    }

    return 0;

    // Suggestion: Validate _LastModified != NULL before dereferencing
}

/** @} */

#endif
