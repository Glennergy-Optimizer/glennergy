/**
 * @file meteo_types.hpp
 * @brief Shared C-compatible data structures for Meteo module.
 *
 * @defgroup MeteoCppModule MeteoCpp Module
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
     *
     * @note Memory ownership:
     * - Owned by PropertyInfo
     *
     * @note Arrays:
     * - time_start: max 31 chars + null terminator
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
     * @brief Weather data for a property.
     *
     * @note Memory ownership:
     * - Fully owned struct, no dynamic allocation
     *
     * @note Arrays:
     * - property_name: METEO_NAME_MAX
     * - sample: KVARTAR_TOTALT
     * - raw_json_data: RAW_DATA_MAX
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
     * @note Memory ownership:
     * - Fully self-contained
     *
     * @note Arrays:
     * - pInfo: PROPERTIES_MAX
     * - Only first pCount elements valid
     */
    typedef struct
    {
        PropertyInfo pInfo[PROPERTIES_MAX];
        size_t pCount;
    } MeteoData;

#ifdef __cplusplus
}
#endif

#endif // METEO_TYPES_HPP