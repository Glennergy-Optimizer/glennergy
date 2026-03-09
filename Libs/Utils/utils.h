#ifndef UTILS_H
#define UTILS_H

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

// Time utils

static uint64_t SystemMonotonicMS()
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

static void GetTodayDate(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d/%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

static void GetTomorrowDate(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d/%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday - 1);
}

static void GetTodayDateFile(char *buffer, size_t size)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d-%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

// Filesystem utils

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
