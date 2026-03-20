/**
 * @file average.h
 * @brief Statistical calculations for Algorithm module.
 * @defgroup Algorithm Algorithm Module
 */

#ifndef AVERAGE_H
#define AVERAGE_H

#include <stddef.h>
#include "../Cache/InputCache.h"
#include "AlgoritmProtocol.h"

/**
 * @brief Generic statistics for numerical data
 * @note Memory owned by parent struct; arrays not used here.
 */
typedef struct {
    double min;     /**< Minimum value */
    double max;     /**< Maximum value */
    double average; /**< Average value */
    double median;  /**< Median value */
    double q25;     /**< 25th percentile */
    double q75;     /**< 75th percentile */
} Stats_t;

/**
 * @brief Meteo statistics combining temperature and global horizontal irradiance
 */
typedef struct {
    Stats_t temperature; /**< Temperature statistics */
    Stats_t ghi;         /**< GHI statistics */
} MeteoStats_t;

/**
 * @brief Spot price statistics per area
 * @note Array size 4 corresponding to SE1-SE4
 */
typedef struct {
    Stats_t area[4]; /**< Area-specific statistics */
} SpotStats_t;

/**
 * @brief Compute statistics for spot prices in cache
 * @param spot Pointer to SpotStats_t to store results
 * @param cache Pointer to InputCache_t with input data
 * @return 0 on success, -1 on invalid parameters
 * @pre `spot` and `cache` must be valid pointers
 * @post `spot` contains min, max, average, median, q25, q75 per area
 * @warning Logs errors to stderr if data missing
 */
int average_SpotprisStats(SpotStats_t *spot, InputCache_t *cache);

/**
 * @brief Detect low-price windows in cache for first area
 * @param cache Pointer to InputCache_t
 * @param q25_threshold Threshold for 25th percentile
 * @return 0 on success, -1 if cache invalid
 * @warning Prints low-price windows to stdout
 */
int average_WindowLow(InputCache_t *cache, double q25_threshold);

/**
 * @brief Test function to determine recommendation based on thresholds
 * @param entry Pointer to SpotEntry_t to evaluate
 * @param q25_threshold Lower threshold
 * @param q75_threshold Upper threshold
 * @return 1 = BUY, 2 = HOLD, 3 = SELL, 0 = invalid, -1 = error
 * @pre `entry` must not be NULL
 * @warning Prints evaluation details to stdout
 */
int average_WindowLow_test(SpotEntry_t *entry, double q25_threshold, double q75_threshold);

#endif