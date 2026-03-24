#ifndef METEO_TYPES_HPP
#define METEO_TYPES_HPP

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KVARTAR_TOTALT 128
#define METEO_NAME_MAX 128
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
    char property_name[METEO_NAME_MAX];
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

#ifdef __cplusplus
}
#endif

#endif // METEO_TYPES_HPP
