#ifndef UTILS_H
#define UTILS_H

// System makron som denna ska komma före headers
// Vi säter alltid posic manuellt med GC Flags vid kompilering
//#define _POSIX_C_SOURCE 200809L 
#define _GNU_SOURCE

// Vanliga headers
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <errno.h> // Den här för create_Folder, resten för system monotonic grejer
#include <string.h>
#include <stdio.h>

// Plattformsepcifika headers 
#if defined(_WIN32)
    #include <windows.h>
#else
    #include <sys/stat.h>
#endif

// Time utils

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

static inline void GetTodayDate(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d/%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

static inline void GetTomorrowDate(char *buffer, size_t size) { //TODO use old GetTodayDate and just add one day to tm struct
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d/%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday - 1);
}

static inline void GetTodayDateFile(char *buffer, size_t size)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d-%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

/**
 * @brief Convert ISO 8601 timestamp with timezone to UTC with 'Z' suffix
 * 
 * @param input Input timestamp like "2026-03-24T15:45:00+01:00"
 * @param output Output buffer (min 32 chars) like "2026-03-24T14:45:00Z"
 * @return 0 on success, -1 on error
 */
static inline int convertToUTC(const char *input, char *output, size_t output_size)
{
    if (!input || !output || output_size < 32)
        return -1;
    
    struct tm tm_time;
    memset(&tm_time, 0, sizeof(tm_time));
    
    int offset_hours = 0, offset_mins = 0;
    char offset_sign = '+';
    
    // Parse: "2026-03-24T15:45:00+01:00"
    int parsed = sscanf(input, "%4d-%2d-%2dT%2d:%2d:%2d%c%2d:%2d",
                       &tm_time.tm_year, &tm_time.tm_mon, &tm_time.tm_mday,
                       &tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec,
                       &offset_sign, &offset_hours, &offset_mins);
    
    if (parsed < 6)
        return -1;  // Failed to parse basic timestamp
    
    // Adjust for struct tm format (year since 1900, month 0-11)
    tm_time.tm_year -= 1900;
    tm_time.tm_mon -= 1;
    
    // Convert to time_t (treats as UTC)
    #if defined(_WIN32)
        time_t utc_time = _mkgmtime(&tm_time);
    #else
        time_t utc_time = timegm(&tm_time);
    #endif

    if (utc_time == -1)
        return -1;
    
    // Apply timezone offset to get actual UTC
    int total_offset_seconds = (offset_hours * 3600) + (offset_mins * 60);
    if (offset_sign == '+')
        utc_time -= total_offset_seconds;  // CET is ahead, subtract to get UTC
    else
        utc_time += total_offset_seconds;  // Negative offset
    
    // Convert back to broken-down time in UTC
    struct tm *utc_tm = gmtime(&utc_time);
    if (!utc_tm)
        return -1;
    
    // Format as ISO 8601 UTC with 'Z'
    snprintf(output, output_size, "%04d-%02d-%02dT%02d:%02d:%02dZ",
             utc_tm->tm_year + 1900,
             utc_tm->tm_mon + 1,
             utc_tm->tm_mday,
             utc_tm->tm_hour,
             utc_tm->tm_min,
             utc_tm->tm_sec);
    
    return 0;
}

typedef enum
{
    DIR_CREATED = 0,
    DIR_ALREADY_EXISTS = 1,
    DIR_ERROR = -1
} dir_result_t;

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
}



#endif
