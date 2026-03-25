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


int Meteo_Initialize(MeteoData *_MeteoData);
int meteo_Fetch(MeteoData *_MeteoData);
int Meteo_LoadGlennergy(MeteoData *_MeteoData);
void Meteo_Dispose(MeteoData* _MeteoData);


#endif // METEO_H
