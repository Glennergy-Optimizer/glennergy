/**
 * @file AlgoritmProtocol.h
 * @brief Shared memory structures and protocol definitions for the Algorithm module.
 *
 * Defines the shared memory layout and basic protocol types used by the Algorithm
 * module.
 *
 * @defgroup Algorithm Algorithm Module
 * @ingroup Algorithm
 * @brief Shared memory structures and protocol definitions for the Algorithm module.
 * @{
 *
 * @note Original comments preserved.
 */

#ifndef TESTREADER_H
#define TESTREADER_H

#define MAX_ID 5                        /**< Maximum number of results in shared memory */
#define ALGORITM_SHARED "/algoritm_shm" /**< Shared memory name */
#define ALGORITM_MUTEX "/algoritm_mutex" /**< Semaphore name for shared memory */

#include "../API/Meteo/Meteo.h"

/**
 * @brief Placeholder function for testing reader.
 *
 * @return 0 on success, -1 on error.
 */
int test_reader();

/**
 * @brief Time representation for a sample.
 *
 * Memory owned by the parent structure; `time` stores up to 32 characters.
 */
typedef struct
{
    char time[32]; /**< Timestamp string (YYYY-MM-DD HH:MM) */
} time_start;

typedef struct{
    float temp[128];
    int weather_code[128];
    int uv_index[128];
}Weather;

/**
 * @brief Result per ID in the Algorithm module.
 *
 * `recommendation` stores values for 96 quarter-hour intervals.
 */
typedef struct
{
    int id;                       /**< Unique identifier */
    double recommendation[96];       /**< Recommendations per quarter-hour */
    double price[96];
    Weather weather;
    time_start time[96];           /**< Corresponding timestamps */
} AlgoritmResult;

/**
 * @brief Shared memory structure for the Algorithm module.
 *
 * Memory ownership is managed by the writer and reader processes.
 */
typedef struct
{
    AlgoritmResult result[MAX_ID]; /**< Array of results */
} AlgoritmShared;

/** @} */

#endif
