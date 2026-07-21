/**
 * @file meteo_types.hpp
 * @brief Shared C-compatible data structures for the Meteo module.
 *
 * @defgroup MeteoCppModule MeteoCpp Module
 * @brief Shared types used by the Meteo C++ implementation.
 *
 * @{
 */

#ifndef METEO_TYPES_HPP
#define METEO_TYPES_HPP

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define KVARTAR_TOTALT 128
#define METEO_NAME_MAX 128
#define RAW_DATA_MAX 16000
#define PROPERTIES_MAX 5

    /**
     * @brief Weather sample for a single 15-minute interval.
     */
    typedef struct
    {
        char time_start[32];
        float temp;
        float diffuse_radiation;
        float cloud_cover;
        int is_day;
        int weather_code;
        int uv_index;
        bool valid;
    } Samples;

    /**
     * @brief Weather data for a single property.
     *
     * Owns only fixed-size storage; no dynamic allocation is used.
     */
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

    /**
     * @brief Container for all properties.
     *
     * Only the first `pCount` entries in `pInfo` are valid.
     */
    typedef struct
    {
        PropertyInfo pInfo[PROPERTIES_MAX];
        size_t pCount;
    } MeteoData;

#ifdef __cplusplus
}
#endif

/** @} */

#endif // METEO_TYPES_HPP
