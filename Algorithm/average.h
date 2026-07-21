/**
 * @file average.h
 * @brief Statistical calculations for the Algorithm module.
 *
 * @defgroup Algorithm Algorithm Module
 * @brief Statistics, window detection, and recommendation helpers for price data.
 */

#ifndef AVERAGE_H
#define AVERAGE_H

#include <stddef.h>
#include "../Cache/InputCache.h"
#include "AlgoritmProtocol.h"

/**
 * @brief Generic statistics for numerical data.
 */
typedef struct {
    double min;     /**< Minimum value. */
    double max;     /**< Maximum value. */
    double average; /**< Average value. */
    double median;  /**< Median value. */
    double q25;     /**< 25th percentile. */
    double q75;     /**< 75th percentile. */
} Stats_t;

/**
 * @brief Meteo statistics combining temperature and global horizontal irradiance.
 */
typedef struct {
    Stats_t temperature; /**< Temperature statistics. */
    Stats_t ghi;         /**< Global Horizontal Irradiance statistics. */
} MeteoStats_t;

/**
 * @brief Spot price statistics per area.
 *
 * The array contains one entry for each Swedish price area, SE1 to SE4.
 */
typedef struct {
    Stats_t area[4]; /**< Area-specific statistics. */
} SpotStats_t;

/**
 * @brief Computes spot price statistics from cache data.
 *
 * @param spot Pointer to the output statistics structure.
 * @param cache Pointer to the input cache.
 *
 * @return 0 on success, -1 on invalid parameters.
 */
int average_SpotprisStats(SpotStats_t *spot, InputCache_t *cache);

/**
 * @brief Detects low-price windows in the first price area.
 *
 * @param cache Pointer to the input cache.
 * @param q25_threshold Threshold used to classify low prices.
 *
 * @return 0 on success, -1 on invalid parameters.
 */
int average_WindowLow(InputCache_t *cache, double q25_threshold);

/**
 * @brief Evaluates one price entry against buy, hold, and sell thresholds.
 *
 * @param entry Pointer to the price entry to evaluate.
 * @param q25_threshold Lower threshold.
 * @param q75_threshold Upper threshold.
 *
 * @return
 * - 1 for BUY
 * - 2 for HOLD
 * - 3 for SELL
 * - 0 for no match
 * - -1 on error
 */
int average_WindowLow_test(SpotEntry_t *entry, double q25_threshold, double q75_threshold);

/**
 * @brief Returns the relative position of a price within a range.
 *
 * @param entry Pointer to the price entry to evaluate.
 * @param min Lower bound of the range.
 * @param max Upper bound of the range.
 *
 * @return Normalized value in the range [0.0, 1.0], or -1.0 on error.
 */
double average_WindowLow_percent(SpotEntry_t *entry, double min, double max);

/**
 * @brief Computes spot price statistics from a Spot_t structure.
 *
 * @param spot Pointer to the output statistics structure.
 * @param entry Pointer to the input spot data.
 *
 * @return 0 on success, -1 on invalid parameters.
 */
int average_SpotprisStats_test(SpotStats_t *spot, Spot_t *entry);

#endif // AVERAGE_H
