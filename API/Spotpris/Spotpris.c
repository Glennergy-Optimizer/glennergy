/**
 * @file Spotpris.c
 * @brief Implementation of the Spotpris module.
 *
 * @ingroup SPOTPRIS
 */

#define MODULE_NAME "SPOTPRIS"
#include "../../Server/Log/Logger.h"
#include "Spotpris.h"
#include "../../Libs/Fetcher.h"
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include "../../Libs/Utils/utils.h"

/**
 * @brief Number of days to fetch.
 *
 * @note Fetches both current and next day.
 */
#define NUM_DAYS 2 // Vi hämtar både dagens och morgondagens data

// Support funktions för internt användning
// Hämta dagens datum i formatet YYYY/MM-DD (URLer måste ha YYYY/MM-DD format)

// Print funktions för debug, kan användas av andra moduler
// SpotPriceEntry
// void SpotPriceEntry_Print(const SpotPriceEntry *e)
// {
//     if (!e) return;
//     printf("  [%s - %s]\n", e->time_start, e->time_end);
//     printf("    SEK: %.5f, EUR: %.5f, EXR: %.5f\n",
//            e->sek_per_kwh, e->eur_per_kwh, e->exchange_rate);
// }

// DagligSpotpris
// void DagligSpotpris_Print(const DagligSpotpris *d)
// {
//     if (!d) return;
//     printf("Area: %s, count: %zu\n", d->areaname, d->count);
//     for (size_t i = 0; i < d->count && i < 4; i++)
//         SpotPriceEntry_Print(&d->kvartar[i]);
//     //printf("Raw JSON (first 20 chars): %.20s\n", d->raw_json_data);
// }

// //OBS: Just nu printar vi 4 kvartar för att debuggingen inte ska bli cluttrad. Kan ändras till 96 om man vill se hela resultatet
// AllaSpotpriser
// void AllaSpotpriser_Print(const AllaSpotpriser *a)
// {
//     if (!a) return;
//     for (int i = 0; i < 4; i++)
//         DagligSpotpris_Print(&a->areas[i]);
// }

/**
 * @brief Implementation of Spotpris_FetchAll.
 *
 * See header for full contract documentation.
 */
int Spotpris_FetchAll(AllaSpotpriser *_AllaSpotpriser)
{
    if (!_AllaSpotpriser)
    {
        return -1;
    }

    char date_str[NUM_DAYS][16];
    for (int i = 0; i < NUM_DAYS; i++)
    {
        if (i == 0)
            GetTodayDate(date_str[i], sizeof(date_str[i]));
        else
            GetTomorrowDate(date_str[i], sizeof(date_str[i]));
    }

    char url[256];
    const char *areas[4] = {"SE1", "SE2", "SE3", "SE4"};

    CurlResponse resp;
    Curl_Initialize(&resp);

    for (int i = 0; i < 4; i++)
    {
        json_t *total_data = json_array();
        for (int k = 0; k < NUM_DAYS; k++)
        {
            resp.size = 0;

            snprintf(url, sizeof(url),
                     "https://www.elprisetjustnu.se/api/v1/prices/%s_%s.json",
                     date_str[k], areas[i]);

            int rc = Curl_HTTPGet(&resp, url);
            if (rc != 0)
            {
                break;
            }

            // Parsa JSON så vi kan skicka med det
            json_error_t error;
            json_t *root = json_loads(resp.data, 0, &error);

            if (!root)
            {
                LOG_ERROR("JSON parse error: %s\n", error.text);
                break;
            }

            if (!json_is_array(root))
            {
                LOG_ERROR("JSON is not an array\n");
                json_decref(root);
                break;
            }

            json_array_extend(total_data, root);
            json_decref(root);
        }

        if (json_array_size(total_data) == 0)
        {
            LOG_ERROR("No data fetched for area %s\n", areas[i]);
            json_decref(total_data);
            continue;
        }

        char *raw_data = json_dumps(total_data, JSON_INDENT(4));
        size_t n = json_array_size(total_data);

        strncpy(_AllaSpotpriser->areas[i].areaname, areas[i], sizeof(_AllaSpotpriser->areas[i].areaname) - 1);
        _AllaSpotpriser->areas[i].areaname[sizeof(_AllaSpotpriser->areas[i].areaname) - 1] = '\0';
        _AllaSpotpriser->areas[i].count = n;

        // Lägg till råa JSON-datan så InputCache kan spara ner det. Glömm inte nullterminering
        strncpy(_AllaSpotpriser->areas[i].raw_json_data, raw_data, sizeof(_AllaSpotpriser->areas[i].raw_json_data) - 1);
        _AllaSpotpriser->areas[i].raw_json_data[sizeof(_AllaSpotpriser->areas[i].raw_json_data) - 1] = '\0';

        // Todo - Kanske onödigt med hårdkodat att vi aldrig läser mer än 96 kvartar, eftersom SpotPris bara kommer leverera 96?
        for (size_t j = 0; j < n; j++)
        {
            json_t *obj = json_array_get(total_data, j);

            if (!json_is_object(obj))
            {
                LOG_ERROR("JSON array element is not an object\n");
                continue;
            }

            const char *start = json_string_value(json_object_get(obj, "time_start"));

            _AllaSpotpriser->areas[i].kvartar[j].sek_per_kwh = json_real_value(json_object_get(obj, "SEK_per_kWh"));

            // Null terminator här också
            strncpy(_AllaSpotpriser->areas[i].kvartar[j].time_start, start, sizeof(_AllaSpotpriser->areas[i].kvartar[j].time_start));
            _AllaSpotpriser->areas[i].kvartar[j].time_start[sizeof(_AllaSpotpriser->areas[i].kvartar[j].time_start) - 1] = '\0';
        }
        LOG_INFO("TEST");
        free(raw_data);
        json_decref(total_data);
    }
    Curl_Dispose(&resp);
    return 0;
}
