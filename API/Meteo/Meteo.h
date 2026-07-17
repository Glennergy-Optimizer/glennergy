/**
 * @file Meteo.h
 * @brief Meteo module for fetching and storing weather forecast data.
 *
 * @defgroup MeteoModule Meteo Module
 *
 * @details
 * This module is responsible for:
 * - Loading property metadata (locations)
 * - Fetching weather data from Open-Meteo API
 * - Storing parsed forecast data (15-min resolution)
 *
 * The module operates on a fixed-size data model and is designed
 * for deterministic, local system execution (no dynamic allocation).
 */

#ifndef METEO_H
#define METEO_H

#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include "../../Libs/Fetcher.h"

/** @brief Maximum number of 15-minute samples (3 days). */
#define KVARTAR_TOTALT 128

/** @brief Maximum length of property name. */
#define NAME_MAX 128

/** @brief Maximum raw JSON buffer size. */
#define RAW_DATA_MAX 16000

/** @brief Maximum number of properties supported. */
#define PROPERTIES_MAX 5

/**
 * @brief Weather sample for a single 15-minute interval.
 *
 * @note Memory ownership: Fully owned by parent PropertyInfo structure.
 * @note Arrays:
 * - time_start: Null-terminated string, max 31 characters + '\0'
 */
typedef struct
{
    char time_start[32];        /**< ISO8601 timestamp */
    float temp;                /**< Temperature in Celsius */              /**< Direct Normal Irradiance (W/m²) */
    float diffuse_radiation;   /**< Diffuse radiation (W/m²) */
    float cloud_cover;         /**< Cloud cover (0–100%) */
    int is_day;                /**< Daylight flag (1 = day, 0 = night) */
    int weather_code;
    int uv_index;
    bool valid;                /**< Valid sample (typically same as is_day) */
} Samples;

/**
 * @brief Weather and metadata for a single property/location.
 *
 * @note Memory ownership:
 * - All buffers are statically allocated and owned by this struct.
 * - No dynamic allocation is used.
 *
 * @note Arrays:
 * - property_name: max length NAME_MAX
 * - sample: fixed size KVARTAR_TOTALT
 * - raw_json_data: max size RAW_DATA_MAX
 * - electricity_area: max 4 chars + null terminator
 */
typedef struct
{
    int id;                                    /**< Unique property ID */
    char property_name[NAME_MAX];               /**< Name of property */
    double lat;                                /**< Latitude */
    double lon;                                /**< Longitude */
    Samples sample[KVARTAR_TOTALT];             /**< Forecast samples */
    char raw_json_data[RAW_DATA_MAX];           /**< Raw API response */
    char electricity_area[5];                   /**< Electricity price area (e.g. SE1–SE4) */
} PropertyInfo;

/**
 * @brief Container for all properties and their weather data.
 *
 * @note Memory ownership:
 * - Fully self-contained structure
 * - No external allocations
 *
 * @note Arrays:
 * - pInfo: fixed size PROPERTIES_MAX
 * - Only first pCount elements are valid
 */
typedef struct
{
    PropertyInfo pInfo[PROPERTIES_MAX]; /**< Property data array */
    size_t pCount;                      /**< Number of active properties */
} MeteoData;

/**
 * @brief Initialize MeteoData structure.
 *
 * @param[out] _MeteoData Pointer to MeteoData
 *
 * @return 0 on success, -1 if input is NULL
 *
 * @pre _MeteoData != NULL
 * @post Structure is initialized with default values
 *
 * @warning Does not clear entire memory via memset, only selected fields
 * @note Safe for repeated calls
 */
int Meteo_Initialize(MeteoData *_MeteoData);

/**
 * @brief Load property metadata from configuration file.
 *
 * @param[out] _MeteoData Pointer to MeteoData
 *
 * @return 0 on success, -1 on failure
 *
 * @pre _MeteoData != NULL
 * @post pCount is updated and property data populated
 *
 * @warning Truncates properties if exceeding PROPERTIES_MAX
 * @note Reads from: /etc/Glennergy-Fastigheter.json
 */
int Meteo_LoadGlennergy(MeteoData *_MeteoData);

/**
 * @brief Fetch and parse weather data for all properties.
 *
 * @param[in,out] _MeteoData Pointer to MeteoData
 *
 * @return
 * - 0 on success
 * - -1 Curl initialization failure
 * - -2 HTTP fetch failure
 * - -3 JSON parsing failure
 *
 * @pre _MeteoData != NULL
 * @pre pCount > 0
 *
 * @post sample[] arrays are populated for each property
 *
 * @warning Network-dependent operation (blocking)
 * @note Uses Fetcher module (Curl wrapper)
 */
int meteo_Fetch(MeteoData *_MeteoData);

/**
 * @brief Reset and clear MeteoData structure.
 *
 * @param[in,out] _MeteoData Pointer to MeteoData
 *
 * @pre _MeteoData may be NULL
 * @post Structure is zeroed
 *
 * @warning Invalidates all previously stored data
 * @note Uses memset for full reset
 */
void Meteo_Dispose(MeteoData *_MeteoData);

#endif // METEO_H
