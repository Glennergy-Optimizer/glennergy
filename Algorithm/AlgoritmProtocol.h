/**
 * @file AlgoritmProtocol.h
 * @brief Shared memory protocol for the Algorithm module.
 *
 * Defines the shared memory layout and protocol types used by the Algorithm
 * module.
 *
 * @defgroup Algorithm Algorithm Module
 * @brief Shared memory protocol for the Algorithm module.
 * @{
 */

#ifndef TESTREADER_H
#define TESTREADER_H

#define MAX_ID 5                        /**< Maximum number of results in shared memory */
#define MAX_FORECAST_ENTRIES 128        /**< Maximum forward quarter-hour entries */
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
 * Stores up to 32 characters in `time`.
 */
typedef struct
{
    char time[32]; /**< Timestamp string (YYYY-MM-DD HH:MM) */
} time_start;

typedef struct{
    float temp[MAX_FORECAST_ENTRIES];
    int weather_code[MAX_FORECAST_ENTRIES];
    int uv_index[MAX_FORECAST_ENTRIES];
}Weather;

/**
 * @brief Result per ID in the Algorithm module.
 *
 * Arrays contain `count` valid forward quarter-hour intervals.
 */
typedef struct
{
    int id;                       /**< Unique identifier */
    size_t count;                 /**< Number of valid entries in each array. */
    double recommendation[MAX_FORECAST_ENTRIES];    /**< Continuous min/max-normalized score. */
    int recommendation_type[MAX_FORECAST_ENTRIES]; /**< 1=buy, 2=hold, 3=sell. */
    double price[MAX_FORECAST_ENTRIES];
    Weather weather;
    time_start time[MAX_FORECAST_ENTRIES]; /**< Corresponding timestamps */
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
