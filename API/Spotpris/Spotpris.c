#define MODULE_NAME "SPOTPRIS"
#include "../../Server/Log/Logger.h"
#include "Spotpris.h"
#include "../../Libs/Fetcher.h"
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../../Libs/Utils/utils.h"


int Spotpris_FetchAll(AllaSpotpriser *_AllaSpotpriser)
{
    if (!_AllaSpotpriser)
    {
        return -1;
    }

    char date_str[16];
    GetTodayDate(date_str, sizeof(date_str));

    char url[256];
    const char *areas[4] = {"SE1", "SE2", "SE3", "SE4"};

    for (int i = 0; i < 4; i++)
    {
        snprintf(url, sizeof(url),
        "https://www.elprisetjustnu.se/api/v1/prices/%s_%s.json",
        date_str, areas[i]);

        CurlResponse resp;
        Curl_Initialize(&resp); 
        
        int rc = Curl_HTTPGet(&resp, url);
        if (rc != 0)
        {
            Curl_Dispose(&resp);
            return rc;
        }
                
        // Parsa JSON så vi kan skicka med det
        json_error_t error;
        json_t *root = json_loads(resp.data, 0, &error);
        
        if (!root)
        {
            LOG_ERROR("JSON parse error: %s\n", error.text);
            Curl_Dispose(&resp);
            return -2;
        }
        
        if (!json_is_array(root))
        {
            LOG_ERROR("JSON is not an array\n");
            json_decref(root);
            Curl_Dispose(&resp);
            return -3;
        }
                
        size_t n = json_array_size(root);
        
        strncpy(_AllaSpotpriser->areas[i].areaname, areas[i], sizeof(_AllaSpotpriser->areas[i].areaname) -1);
        _AllaSpotpriser->areas[i].areaname[sizeof(_AllaSpotpriser->areas[i].areaname) -1] = '\0';
        _AllaSpotpriser->areas[i].count = n;
        
        // Lägg till råa JSON-datan så InputCache kan spara ner det. Glömm inte nullterminering
        strncpy(_AllaSpotpriser->areas[i].raw_json_data, resp.data, sizeof(_AllaSpotpriser->areas[i].raw_json_data) -1);
        _AllaSpotpriser->areas[i].raw_json_data[sizeof(_AllaSpotpriser->areas[i].raw_json_data) -1] = '\0';
        
        // Todo - Kanske onödigt med hårdkodat att vi aldrig läser mer än 96 kvartar, eftersom SpotPris bara kommer leverera 96?
        for (size_t j = 0; j < n && j < 96; j++) 
        {
            json_t *obj = json_array_get(root, j);

            const char *start = json_string_value(json_object_get(obj, "time_start"));
            //const char *end = json_string_value(json_object_get(obj, "time_end"));
            _AllaSpotpriser->areas[i].kvartar[j].sek_per_kwh = json_real_value(json_object_get(obj, "SEK_per_kWh"));
            //_AllaSpotpriser->areas[i].kvartar[j].eur_per_kwh = json_real_value(json_object_get(obj, "EUR_per_kWh"));
            //_AllaSpotpriser->areas[i].kvartar[j].exchange_rate = json_real_value(json_object_get(obj, "EXR"));
            
            char utc_timestamp[32];
            if (convertToUTC(start, utc_timestamp, sizeof(utc_timestamp)) == 0) {
                strncpy(_AllaSpotpriser->areas[i].kvartar[j].time_start, utc_timestamp, sizeof(_AllaSpotpriser->areas[i].kvartar[j].time_start));
            } else {
                LOG_WARNING("Failed to convert timestamp to UTC: %s\n", start);
                strncpy(_AllaSpotpriser->areas[i].kvartar[j].time_start, start, sizeof(_AllaSpotpriser->areas[i].kvartar[j].time_start));
            }
            _AllaSpotpriser->areas[i].kvartar[j].time_start[sizeof(_AllaSpotpriser->areas[i].kvartar[j].time_start) -1] = '\0';
            //strncpy(_AllaSpotpriser->areas[i].kvartar[j].time_end, end, sizeof(_AllaSpotpriser->areas[i].kvartar[j].time_end));
            //_AllaSpotpriser->areas[i].kvartar[j].time_end[sizeof(_AllaSpotpriser->areas[i].kvartar[j].time_end) -1] = '\0';
        }
        
        Curl_Dispose(&resp);
        json_decref(root);
    }

    return 0;
}
