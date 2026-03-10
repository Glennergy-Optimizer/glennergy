#ifndef SOLAR_H
#define SOLAR_H

#include "../Cache/CacheProtocol.h"

int solar_PredictHome(CacheData_t *cache, int home_idx, double *solar_output);

#endif