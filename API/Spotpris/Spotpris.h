/**
 * @file Spotpris.h
 * @brief Public API for the Spotpris module.
 *
 * Provides data structures and functions for fetching and handling
 * electricity spot prices from an external API.
 */

#ifndef SPOTPRIS_H
#define SPOTPRIS_H

#include <stddef.h>

/**
 * @defgroup SPOTPRIS Spotpris
 * @brief Handling of electricity spot prices via external API.
 *
 * This module fetches, parses, and stores spot price data for Swedish
 * electricity regions (SE1–SE4).
 *
 * @note Data includes both current and next day's prices.
 * @note Internally performs network I/O and JSON parsing.
 * @{
 */

/**
 * @brief Represents a single spot price entry.
 *
 * Contains price data for a specific time interval.
 */
typedef struct {
    char time_start[32]; /**< Start time in ISO format, null-terminated. */
    //char time_end[32];  /**< End time in ISO format. */
    double sek_per_kwh;  /**< Price in SEK per kWh. */
    //double eur_per_kwh; /**< Price in EUR per kWh. */
    //double exchange_rate; /**< Exchange rate used. */
} SpotPriceEntry;

/**
 * @brief Represents spot prices for a single electricity area.
 *
 * Contains both parsed data and the raw JSON response for that area.
 */
typedef struct
{
    char areaname[4];       /**< Electricity area (e.g. "SE1", "SE2", "SE3", "SE4"). */
    size_t count;           /**< Number of valid entries in kvartar. */
    SpotPriceEntry kvartar[192]; /**< Spot price entries; only first `count` are valid. */
    char raw_json_data[32000]; /**< Raw JSON data from API, possibly truncated and null-terminated. */ // Vid ett test av spotprisdatan var den 13600 

} DagligSpotpris;

/**
 * @brief Collection of spot prices for all electricity areas.
 *
 * Contains one DagligSpotpris per Swedish electricity area (SE1–SE4).
 */
typedef struct 
{
    DagligSpotpris areas[4]; /**< Daily spot prices for "SE1", "SE2", "SE3", "SE4". */
} AllaSpotpriser;


// Print / debug functions

/**
 * @brief Prints a SpotPriceEntry for debugging.
 *
 * @param e Pointer to entry to print (may be NULL).
 */
void SpotPriceEntry_Print(const SpotPriceEntry *e);

/**
 * @brief Prints a DagligSpotpris for debugging.
 *
 * @param d Pointer to daily spot price data (may be NULL).
 */
void DagligSpotpris_Print(const DagligSpotpris *d);

/**
 * @brief Prints all spot prices for debugging.
 *
 * @param a Pointer to all spot price data (may be NULL).
 */
void AllaSpotpriser_Print(const AllaSpotpriser *a);

/**
 * @brief Fetches spot prices for all electricity areas (SE1–SE4).
 *
 * Fetches data for both current and next day from an external API,
 * parses JSON, and populates the provided structure.
 *
 * @param[out] _AllaSpotpriser Output structure to populate (must be pre-allocated).
 *
 * @return
 * - 0 on success (may contain partial data if some areas fail)
 * - -1 on critical failure (e.g. NULL pointer)
 *
 * @note Performs blocking network I/O and JSON parsing.
 * @note Issues up to 8 HTTP requests per call (4 areas × 2 days).
 * @note Logs errors via the Logger module.
 */
int Spotpris_FetchAll(AllaSpotpriser *_AllaSpotpriser);

/** @} */

#endif
