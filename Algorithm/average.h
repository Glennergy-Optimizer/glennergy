#ifndef AVERAGE_H
#define AVERAGE_H

#include <stddef.h>

#include "../Cache/InputCache.h"
#include "testreader.h"


typedef struct {
    double min;
    double max;
    double average;
    double median;
    double q25;
    double q75;
} Stats_t;

typedef struct {
    Stats_t temperature;
    Stats_t ghi;
}   MeteoStats_t;

typedef struct {
    Stats_t area[4];
} SpotStats_t;


int average_SpotprisStats(SpotStats_t *spot, CacheData_t *cache);
int average_WindowLow(CacheData_t *cache, double q25_threshold);
int average_WindowLow_test(SpotEntry_t *entry, double q25_threshold, double q75_threshold);

#endif