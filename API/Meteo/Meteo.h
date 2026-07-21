/**
 * @file Meteo.h
 * @brief Public API for the Meteo module.
 *
 * @defgroup MeteoModule MeteoModule
 * @brief Weather data fetching and storage.
 *
 * Loads property metadata, fetches Open-Meteo forecasts, and stores parsed
 * samples in fixed-size structures with no dynamic allocation.
 * @{
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
 * @brief Weather sample for one 15-minute interval.
 *
 * @note Owned by the enclosing PropertyInfo structure.
 */
typedef struct
{
    char time_start[32];        /**< ISO8601 timestamp. */
    float temp;                 /**< Temperature in Celsius. */
    float diffuse_radiation;    /**< Diffuse radiation (W/m²). */
    float cloud_cover;          /**< Cloud cover (0–100%). */
    int is_day;                /**< Daylight flag (1 = day, 0 = night). */
    int weather_code;          /**< Weather code. */
    int uv_index;              /**< UV index. */
    bool valid;                /**< Valid sample flag. */
} Samples;

/**
 * @brief Weather and metadata for one property or location.
 *
 * All buffers are fixed size and owned by this structure.
 */
typedef struct
{
    int id;                                     /**< Unique property ID. */
    char property_name[NAME_MAX];               /**< Property name. */
    double lat;                                 /**< Latitude. */
    double lon;                                 /**< Longitude. */
    Samples sample[KVARTAR_TOTALT];             /**< Forecast samples. */
    char raw_json_data[RAW_DATA_MAX];           /**< Raw API response. */
    char electricity_area[5];                   /**< Electricity price area. */
} PropertyInfo;

/**
 * @brief Container for all configured properties and their weather data.
 *
 * Only the first pCount entries in pInfo are valid.
 */
typedef struct
{
    PropertyInfo pInfo[PROPERTIES_MAX]; /**< Property data array. */
    size_t pCount;                      /**< Number of active properties. */
} MeteoData;

/**
 * @brief Initializes a MeteoData structure.
 *
 * @param[out] _MeteoData Pointer to the structure to initialize.
 *
 * @return 0 on success, -1 if _MeteoData is NULL.
 */
int Meteo_Initialize(MeteoData *_MeteoData);

/**
 * @brief Loads property metadata from the configuration file.
 *
 * @param[out] _MeteoData Pointer to the destination structure.
 *
 * @return 0 on success, -1 on failure.
 */
int Meteo_LoadGlennergy(MeteoData *_MeteoData);

/**
 * @brief Fetches and parses weather data for all configured properties.
 *
 * @param[out] _MeteoData Pointer to the destination structure.
 *
 * @return
 * - 0 on success
 * - -1 on Curl initialization failure
 * - -2 on HTTP fetch failure
 * - -3 on JSON parsing failure
 */
int meteo_Fetch(MeteoData *_MeteoData);

/**
 * @brief Resets a MeteoData structure.
 *
 * @param[in,out] _MeteoData Pointer to the structure to clear.
 */
void Meteo_Dispose(MeteoData *_MeteoData);

/** @} */

#endif // METEO_H
