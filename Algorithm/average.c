// #define MODULE_NAME "ALGOINFLUENCER"
// #include "algoinfluencer.h"
// #include "../../server/log/logger.h"
#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
// #include <sys/stat.h>
#include "average.h"
#include "../Cache/InputCache.h"

const char *area_names[AREA_COUNT] = {"SE1", "SE2", "SE3", "SE4"}; // usch

int compare_double(const void *a, const void *b)
{
    double diff = (*(double *)a - *(double *)b);
    if (diff < 0)
        return -1; // a < b
    if (diff > 0)
        return 1; // a > b
    return 0;     // a == b
}

int average_SpotprisStats(SpotStats_t *spot, InputCache_t *cache)
{
    if (!cache || !spot)
    {
        fprintf(stderr, "Invalid parameters\n");
        return -1;
    }

    for (int areaindx = 0; areaindx < AREA_COUNT; areaindx++)
    {
        size_t count = cache->spotpris.count[areaindx];

        printf("COUNT %zu\n", count);
        if (count == 0)
        {
            fprintf(stderr, "No data for area %s\n", area_names[areaindx]);
            continue;
        }

        double sorted[192];
        double sum = 0.0;

        for (size_t samples = 0; samples < count; samples++)
        {
            sorted[samples] = cache->spotpris.data[areaindx][samples].sek_per_kwh;
            sum += sorted[samples];
        }

        qsort(sorted, count, sizeof(double), compare_double);

        size_t mid = count / 2;
        double median = (count % 2 == 0)
                            ? (sorted[mid - 1] + sorted[mid]) / 2.0
                            : sorted[mid];

        spot->area[areaindx].min = sorted[0];
        spot->area[areaindx].max = sorted[count - 1];
        spot->area[areaindx].average = sum / count;
        spot->area[areaindx].median = median;
        spot->area[areaindx].q25 = sorted[count / 4];
        spot->area[areaindx].q75 = sorted[(3 * count) / 4];

        printf("\nArea %s: min=%.3f, max=%.3f, avg=%.3f, median=%.3f, q25=%.3f, q75=%.3f\n",
               area_names[areaindx],
               spot->area[areaindx].min,
               spot->area[areaindx].max,
               spot->area[areaindx].average,
               spot->area[areaindx].median,
               spot->area[areaindx].q25,
               spot->area[areaindx].q75);
    }

    return 0;
}

int average_WindowLow(InputCache_t *cache, double q25_threshold)
{
    if (!cache)
    {
        fprintf(stderr, "Invalid cache parameter\n");
        return -1;
    }

    printf(" q25_threshold: %.3f\n", q25_threshold);
    int start = -1;
    for (size_t i = 0; i < cache->spotpris.count[0]; i++)
    {
        double price = cache->spotpris.data[0][i].sek_per_kwh;

        if (price < q25_threshold)
        {
            if (start == -1)
            {
                start = i; // Start of a low-price window
            }
        }
        else
        {
            if (start != -1)
            {
                printf(" Low price window: %s (%.3f SEK/kWh)\n\t\tto %s (%.3f SEK/kWh)\n",
                       cache->spotpris.data[0][start].time_start,
                       cache->spotpris.data[0][start].sek_per_kwh,
                       cache->spotpris.data[0][i - 1].time_start,
                       cache->spotpris.data[0][i - 1].sek_per_kwh);
                start = -1; // Reset for next window
            }
        }
    }
    if (start != -1)
    {
        printf(" Low price window: %s (%.3f SEK/kWh)\n\t\tto %s (%.3f SEK/kWh)\n",
               cache->spotpris.data[0][start].time_start,
               cache->spotpris.data[0][start].sek_per_kwh,
               cache->spotpris.data[0][cache->spotpris.count[0] - 1].time_start,
               cache->spotpris.data[0][cache->spotpris.count[0] - 1].sek_per_kwh);
    }

    return 0;
}

int average_WindowLow_test(SpotEntry_t *entry, double q25_threshold, double q75_threshold)
{
    if (!entry)
    {
        fprintf(stderr, "Invalid cache parameter\n");
        return -1;
    }

    printf(" q25_threshold: %.3f\n", q25_threshold);

    double price = entry->sek_per_kwh;

    if (price < q25_threshold)
    {
        printf("Price below q25: %s (%.3f SEK/kWh) [BUY]\n", entry->time_start, price);
        return 1;
    }

    if (price >= q25_threshold && price < q75_threshold)
    {
        printf("Price between q25 and q75: %s (%.3f SEK/kWh) [HOLD]\n", entry->time_start, price);
        return 2;
    }

    if (price >= q75_threshold)
    {
        printf("Price above q75: %s (%.3f SEK/kWh) [SELL]\n", entry->time_start, price);
        return 3;
    }

    return 0;
}

int average_SpotprisStats_test(SpotStats_t *spot, Spot_t *entry)
{
    if (!entry || !spot)
    {
        fprintf(stderr, "Invalid parameters\n");
        return -1;
    }

    for (int areaindx = 0; areaindx < AREA_COUNT; areaindx++)
    {
        size_t count = entry->count[areaindx];

        if (count == 0)
        {
            fprintf(stderr, "No data for area %s\n", area_names[areaindx]);
            continue;
        }

        double sorted[96];
        double sum = 0.0;

        for (size_t samples = 0; samples < count; samples++)
        {
            sorted[samples] = entry->data[areaindx][samples].sek_per_kwh;
            sum += sorted[samples];
        }

        qsort(sorted, count, sizeof(double), compare_double);

        size_t mid = count / 2;
        double median = (count % 2 == 0)
                            ? (sorted[mid - 1] + sorted[mid]) / 2.0
                            : sorted[mid];

        spot->area[areaindx].min = sorted[0];
        spot->area[areaindx].max = sorted[count - 1];
        spot->area[areaindx].average = sum / count;
        spot->area[areaindx].median = median;
        spot->area[areaindx].q25 = sorted[count / 4];
        spot->area[areaindx].q75 = sorted[(3 * count) / 4];

        printf("\nArea %s: min=%.3f, max=%.3f, avg=%.3f, median=%.3f, q25=%.3f, q75=%.3f\n",
               area_names[areaindx],
               spot->area[areaindx].min,
               spot->area[areaindx].max,
               spot->area[areaindx].average,
               spot->area[areaindx].median,
               spot->area[areaindx].q25,
               spot->area[areaindx].q75);
    }

    return 0;
}