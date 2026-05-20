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
 *
 * @note All string fields are null-terminated.
 */
typedef struct {
    char time_start[32]; /**< Start time in ISO format. */
    //char time_end[32];  /**< End time in ISO format. */
    double sek_per_kwh;  /**< Price in SEK per kWh. */
    //double eur_per_kwh; /**< Price in EUR per kWh. */
    //double exchange_rate; /**< Exchange rate used. */
} SpotPriceEntry;

/**
 * @brief Represents spot prices for a single electricity area.
 *
 * Contains both parsed data and raw JSON response.
 *
 * @note Raw JSON data may be truncated if exceeding buffer size.
 */
typedef struct
{
    char areaname[4]; /**< Electricity area (e.g. "SE1", "SE2", "SE3", "SE4"). */
    size_t count; /**< Number of valid entries in kvartar. */
    SpotPriceEntry kvartar[192]; /**< Entries; only first `count` are valid. */
    char raw_json_data[32000]; /**< Raw JSON data from API. */ // Vid ett test av spotprisdatan var den 13600 

} DagligSpotpris;

/**
 * @brief Collection of spot prices for all electricity areas.
 *
 * @note Contains data for SE1–SE4.
 */
typedef struct 
{
    DagligSpotpris areas[4]; /**< "SE1", "SE2", "SE3", "SE4" */
} AllaSpotpriser;


// Print / debug functions

/**
 * @brief Prints a SpotPriceEntry (debug).
 *
 * @param e Pointer to entry to print.
 */
void SpotPriceEntry_Print(const SpotPriceEntry *e);

/**
 * @brief Prints a DagligSpotpris (debug).
 *
 * @param d Pointer to daily spot price data.
 */
void DagligSpotpris_Print(const DagligSpotpris *d);

/**
 * @brief Prints all spot prices (debug).
 *
 * @param a Pointer to all spot price data.
 */
void AllaSpotpriser_Print(const AllaSpotpriser *a);

/**
 * @brief Fetches spot prices for all electricity areas (SE1–SE4).
 *
 * Fetches data for both current and next day from an external API,
 * parses JSON, and populates the provided structure.
 *
 * @param[out] _AllaSpotpriser Output structure to populate.
 *
 * @return
 * - 0 on success (may contain partial data if some areas fail)
 * - -1 on critical failure (e.g. NULL pointer)
 *
 * @note Performs blocking network I/O.
 * @note Logs errors via Logger module.
 *
 * @pre _AllaSpotpriser must not be NULL.
 */
int Spotpris_FetchAll(AllaSpotpriser *_AllaSpotpriser);

/** @} */

#endif
