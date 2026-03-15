#define MODULE_NAME "METEO"
#include "../../Server/Log/Logger.h"
#include "Meteo.h"
#include "../../Libs/Fetcher.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <jansson.h>

#define METEO_LINK "https://api.open-meteo.com/v1/forecast?latitude=%2.f&longitude=%2f&minutely_15=temperature_2m,shortwave_radiation,direct_normal_irradiance,diffuse_radiation,cloud_cover,is_day&forecast_days=3&forecast_minutely_15=128&timezone=Europe/Stockholm"

int Meteo_Initialize(MeteoData *_MeteoData)
{
    if (_MeteoData == NULL)
        return -1;

    _MeteoData->pCount = 0;

    for (int i = 0; i < PROPERTIES_MAX; i++)
    {
        _MeteoData->pInfo[i].id = -1;
        _MeteoData->pInfo[i].lat = 0.0;
        _MeteoData->pInfo[i].lon = 0.0;
        _MeteoData->pInfo[i].property_name[0] = '\0';
        _MeteoData->pInfo[i].raw_json_data[0] = '\0';
    }

    return 0;
}

int Meteo_LoadGlennergy(MeteoData *_MeteoData)
{
    json_error_t err;
    json_t *root = json_load_file("/etc/Glennergy-Fastigheter.json", 0, &err);

    if (root == NULL)
    {
        LOG_ERROR("failed to load file!\n");
        return -1;
    }
    json_t *property = json_object_get(root, "systems");
    if (!json_is_array(property))
    {
        LOG_ERROR("'systems' is not an array!\n");
        json_decref(root);
        return -1;
    }
    
    _MeteoData->pCount = json_array_size(property);

    if (_MeteoData->pCount > PROPERTIES_MAX)
    {
        LOG_WARNING("Too many properties in JSON, truncating to %d\n", PROPERTIES_MAX);
        _MeteoData->pCount = PROPERTIES_MAX;
    }

    for (size_t i = 0; i < _MeteoData->pCount; i++)
    {
        json_t *object = json_array_get(property, i);

        //LOG_INFO("Loading property %zu/%zu\n", i + 1, _MeteoData->pCount);

        if (!json_is_object(object))
            continue;



        const char *city;
        int result = json_unpack(object, "{s:i, s:s, s:f, s:f}",
                                "id", &_MeteoData->pInfo[i].id,
                                "city", &city,
                                "lat", &_MeteoData->pInfo[i].lat,
                                "lon", &_MeteoData->pInfo[i].lon);

        if (result != 0)
            continue;

        snprintf(_MeteoData->pInfo[i].property_name, NAME_MAX, "%s", city);

        LOG_INFO("Loaded property: %s (ID: %d, Lat: %.2f, Lon: %.2f)\n", _MeteoData->pInfo[i].property_name, _MeteoData->pInfo[i].id, _MeteoData->pInfo[i].lat, _MeteoData->pInfo[i].lon);
    }

    json_decref(root);

    return 0;
}

int Meteo_Parse(MeteoData *_MeteoData, const char *_JsonRaw)
{

    json_error_t error;
    json_t *root = json_loads(_JsonRaw, 0, &error);
    if (!root)
    {
        LOG_ERROR("JSON parse failed: %s\n", error.text);
        return -1;
    }

    json_t *hourly = json_object_get(root, "minutely_15");
    if (!hourly || !json_is_object(hourly))
    {
        LOG_ERROR("JSON missing 'minutely_15' object\n");
        json_decref(root);
        return -2;
    }
    json_t *times = json_object_get(hourly, "time");
    json_t *temps = json_object_get(hourly, "temperature_2m");
    json_t *ghi = json_object_get(hourly, "shortwave_radiation");
    json_t *dni = json_object_get(hourly, "direct_normal_irradiance");
    json_t *diffuse = json_object_get(hourly, "diffuse_radiation");
    json_t *cloud_cover = json_object_get(hourly, "cloud_cover");
    json_t *is_day = json_object_get(hourly, "is_day");

    size_t array_size = json_array_size(temps);

    for (size_t i = 0; i < _MeteoData->pCount; i++)
    {

        for (size_t j = 0; j < array_size; j++)
        {
            snprintf(_MeteoData->pInfo[i].sample[j].time_start,
            sizeof(_MeteoData->pInfo[i].sample[j].time_start), "%s", json_string_value(json_array_get(times, j)));
            
            _MeteoData->pInfo[i].sample[j].temp = json_number_value(json_array_get(temps, j));
            _MeteoData->pInfo[i].sample[j].ghi = json_number_value(json_array_get(ghi, j));
            _MeteoData->pInfo[i].sample[j].dni = json_number_value(json_array_get(dni, j));
            _MeteoData->pInfo[i].sample[j].diffuse_radiation = json_number_value(json_array_get(diffuse, j));
            _MeteoData->pInfo[i].sample[j].cloud_cover = json_number_value(json_array_get(cloud_cover, j));
            _MeteoData->pInfo[i].sample[j].is_day = json_integer_value(json_array_get(is_day, j)) != 0;
            _MeteoData->pInfo[i].sample[j].valid = _MeteoData->pInfo[i].sample[j].is_day;
        }

        snprintf(_MeteoData->pInfo[i].raw_json_data, RAW_DATA_MAX, "%s", _JsonRaw);
    }

    json_decref(root);
    return 0;
}

int meteo_Fetch(MeteoData *_MeteoData)
{

    for (size_t i = 0; i < _MeteoData->pCount; i++)
    {

        CurlResponse response;
        int curl_result = Curl_Initialize(&response);

        if (curl_result < 0)
        {
            LOG_WARNING("Curl initialization failed with code %d\n", curl_result);
            return -1;
        }

        char url[512];
        snprintf(url, sizeof(url), METEO_LINK, _MeteoData->pInfo[i].lat, _MeteoData->pInfo[i].lon);


        LOG_INFO("URL: %s\n", url);

        // Use Fetcher to get the data
        int fetch_result = Curl_HTTPGet(&response, url);
        if (fetch_result != 0 || response.data == NULL)
        {
            LOG_WARNING("HTTP fetch failed with code %d\n", fetch_result);
            Curl_Dispose(&response);
            return -2;
        }

        // printf("%s\n", response.data);
        int parse_result = Meteo_Parse(_MeteoData, response.data);

        if (parse_result < 0)
        {
            Curl_Dispose(&response);
            return -3;
        }

        Curl_Dispose(&response);
    }
    return 0;
}

void Meteo_Dispose(MeteoData *_MeteoData)
{
    if (_MeteoData == NULL)
    {
        return;
    }

    memset(_MeteoData, 0, sizeof(MeteoData));
}