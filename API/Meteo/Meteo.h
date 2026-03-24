
/**

* @file Meteo.h

* @brief Weather data retrieval and parsing module using Open-Meteo API.


* This module is responsible for:

* Initializing and managing meteorological data structures

* Loading property configuration from a JSON file
 
* Fetching weather forecast data from the Open-Meteo API

* Parsing JSON responses into structured data

* The module supports multiple properties (locations), each identified by

* geographic coordinates and metadata. Forecast data is retrieved in

* 15-minute intervals and includes temperature, solar radiation metrics,

* cloud cover, and day/night indicators.


* External dependencies:

* libcurl (via Curl_* wrapper functions)

* jansson (JSON parsing)

* Logger module


* Configuration source:

* /etc/Glennergy-Fastigheter.json


* API endpoint:

* https://api.open-meteo.com/


*@defgroup MeteoModule Meteo Module

@{
*/


#ifndef METEO_H
#define METEO_H

#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include "../../Libs/Fetcher.h"
/**
 * Fetch weather forecast from Open-Meteo API
 *
 * @param lat Latitude
 * @param lon Longitude
 * @param weather_out Output array for hourly weather data
 * @param max_hours Maximum hours to fetch (up to 72)
 * @return Number of hours fetched, or -1 on error
 */
// Weather data from API (hourly)

#define KVARTAR_TOTALT 128
#define NAME_MAX 128
#define RAW_DATA_MAX 16000
#define PROPERTIES_MAX 5

typedef struct
{
    char time_start[32];
    float temp;              // Celsius
    float ghi;               // Global Horizontal Irradiance (W/m²) - calculated from DNI + diffuse
    float dni;               // Direct Normal Irradiance (W/m²)
    float diffuse_radiation; // Diffuse Radiation (W/m²)
    float cloud_cover;       // Cloud cover percentage (0-100%)
    int is_day;
    bool valid;
} Samples;

typedef struct
{
    int id;
    char property_name[NAME_MAX];
    double lat;
    double lon;
    Samples sample[KVARTAR_TOTALT];
    char raw_json_data[RAW_DATA_MAX];
    char electricity_area[5];
} PropertyInfo;

typedef struct
{
    PropertyInfo pInfo[PROPERTIES_MAX];
    size_t pCount;
} MeteoData;

/**

* @brief Initialize MeteoData structure.

* Resets all fields in the MeteoData structure and prepares it for use.

* @param[in,out] _MeteoData Pointer to MeteoData structure.

* @return 0 on success, -1 if input pointer is NULL.
*/
int Meteo_Initialize(MeteoData *_MeteoData);

/** 

* @brief Fetch weather data for all configured properties.


* Iterates through all properties in MeteoData, builds API requests,

* retrieves forecast data using HTTP, and parses the results.


* @param[in,out] _MeteoData Pointer to MeteoData structure.


* @return 0 on success, negative value on failure:

* @retval -1 Curl initialization failure

* @retval -2 HTTP fetch failure

* @retval -3 Parsing failure
*/
int meteo_Fetch(MeteoData *_MeteoData);

/**

* @brief Load property configuration from Glennergy JSON file.


* Reads property information such as ID, city name, latitude,

* longitude, and electricity area from a configuration file.


* @param[in,out] _MeteoData Pointer to MeteoData structure.

* @return 0 on success, negative value on failure:

* @retval -1 File load or JSON format error
*/
int Meteo_LoadGlennergy(MeteoData *_MeteoData);

/** 

* @brief Release resources and reset MeteoData structure.


* Clears all data in the MeteoData structure.


* @param[in,out] _MeteoData Pointer to MeteoData structure.
*/
void Meteo_Dispose(MeteoData* _MeteoData);


#endif // METEO_H
