/**
 * @file Spotpris.h
 * @brief Public API for the Spotpris module.
 *
 * Defines the data structures and functions used to fetch and inspect
 * electricity spot prices from the external API.
 */

#ifndef SPOTPRIS_H
#define SPOTPRIS_H

#include <stddef.h>

/**
 * @defgroup SPOTPRIS Spotpris
 * @brief Electricity spot price handling.
 *
 * Fetches, parses, stores, and optionally prints spot price data for the
 * Swedish electricity areas SE1 to SE4.
 *
 * @note Fetching performs blocking network I/O and JSON parsing.
 * @{
 */

/**
 * @brief Represents a single spot price entry.
 *
 * Contains price data for one time interval.
 */
typedef struct {
    char time_start[32]; /**< Start time in ISO format. */
    //char time_end[32];  /**< End time in ISO format. */
    double sek_per_kwh;  /**< Price in SEK per kWh. */
    //double eur_per_kwh; /**< Price in EUR per kWh. */
    //double exchange_rate; /**< Exchange rate used. */
} SpotPriceEntry;

/**
 * @brief Spot prices for one electricity area.
 *
 * Holds the parsed entries together with the raw JSON response returned by the
 * API.
 *
 * @note Owns its stored data.
 * @note Raw JSON may be truncated if it exceeds the buffer size.
 */
typedef struct
{
    char areaname[4]; /**< Electricity area, for example "SE1". */
    size_t count; /**< Number of valid entries in kvartar. */
    SpotPriceEntry kvartar[192]; /**< Parsed entries; only the first count are valid. */
    char raw_json_data[32000]; // Vid ett test av spotprisdatan var den 13600 

} DagligSpotpris;

/**
 * @brief Spot prices for all electricity areas.
 *
 * Contains one daily dataset per area.
 *
 * @note Owns all nested data.
 */
typedef struct 
{
    DagligSpotpris areas[4]; /**< Areas in order: "SE1", "SE2", "SE3", "SE4". */
} AllaSpotpriser;


// Print / debug functions

/**
 * @brief Prints a SpotPriceEntry for debugging.
 *
 * @param e Pointer to the entry to print.
 */
void SpotPriceEntry_Print(const SpotPriceEntry *e);

/**
 * @brief Prints a DagligSpotpris for debugging.
 *
 * @param d Pointer to the daily spot price data.
 */
void DagligSpotpris_Print(const DagligSpotpris *d);

/**
 * @brief Prints all spot prices for debugging.
 *
 * @param a Pointer to all spot price data.
 */
void AllaSpotpriser_Print(const AllaSpotpriser *a);

/**
 * @brief Fetches spot prices for all electricity areas.
 *
 * Fetches data for both the current and next day, parses the JSON response,
 * and populates the provided structure.
 *
 * @param[out] _AllaSpotpriser Output structure to populate.
 *
 * @return
 * - 0 on success
 * - -1 on critical failure, such as a NULL output pointer
 *
 * @note Performs blocking network I/O.
 * @note Up to 8 HTTP requests are made per call.
 * @note Logs errors via the Logger module.
 */
int Spotpris_FetchAll(AllaSpotpriser *_AllaSpotpriser);

/** @} */

#endif
