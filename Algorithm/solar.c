#define MODULE_NAME "SOLAR"
#include "../Server/Log/Logger.h"
#include "solar.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PANEL_EFFICIENCY 0.18           // 18% conversion efficiency
#define TEMP_DERATING_FACTOR 0.005      // -0.5% per degree C above 25°C
#define TEMP_DERATING_MIN 0.8           // Minimum 80% efficiency
#define TEMP_DERATING_MAX 1.0           // Maximum 100% efficiency
#define REFERENCE_TEMP_C 25.0           // Reference temperature
#define CLOUD_REDUCTION_FACTOR 0.75     // Max 75% reduction from clouds
#define TILT_BONUS_FACTOR 0.2           // 20% max bonus from tilt
#define QUARTERS_PER_HOUR 4.0           // 15-minute intervals

static double solar_CalculateSunEfficiency(int time_slot, double lat, double panel_tilt)
{
    (void)lat;
    // Hour of day (0-24)
    double hour = (time_slot / QUARTERS_PER_HOUR);
    
    // Hour angle from solar noon
    double hour_angle = (hour - 12.0) * 15.0;
    
    // Sun elevation (simplified)
    double efficiency = cos(hour_angle * M_PI / 180.0);
    
    if (efficiency < 0) return 0.0;
    
    // Tilt bonus (simplified)
    double tilt_factor = 1.0 + (panel_tilt / 90.0) * TILT_BONUS_FACTOR;
    
    return efficiency * tilt_factor;
}

static double solar_PredictSlot(double ghi, double cloud_cover, double temp, double panel_capacity, double panel_tilt, double lat, int time_slot, int is_day)
{
    if (time_slot < 0 || time_slot >= 96)
        return 0.0;

    if(!is_day)
        return 0.0;
    
    // Cloud reduction
    double cloud_factor = 1.0 - (cloud_cover / 100.0 * CLOUD_REDUCTION_FACTOR);
    double effective_ghi = ghi * cloud_factor;
    
    // Sun angle
    double sun_efficiency = solar_CalculateSunEfficiency(time_slot, lat, panel_tilt);
    
    // Temperature derating
    double temp_factor = 1.0 - ((temp - REFERENCE_TEMP_C) * TEMP_DERATING_FACTOR);
    if (temp_factor < TEMP_DERATING_MIN) temp_factor = TEMP_DERATING_MIN;
    if (temp_factor > TEMP_DERATING_MAX) temp_factor = TEMP_DERATING_MAX;
    
    // Panel efficiency assumption
    double panel_efficiency = PANEL_EFFICIENCY;
    
    // kWh for 15-min interval
    double solar_kwh = (effective_ghi / 1000.0) * panel_efficiency * 
                       sun_efficiency * temp_factor * panel_capacity / QUARTERS_PER_HOUR;
    
    return (solar_kwh > 0.0) ? solar_kwh : 0.0;
}

int solar_PredictHome(CacheData_t *cache, int home_idx, double *solar_output)
{
    if (!cache || home_idx < 0 || home_idx >= (int)cache->home_count || !solar_output) {
        LOG_ERROR("Invalid parameters");
        return -1;
    }
    
    // Find matching meteo data for this home
    int meteo_idx = -1;
    for (size_t i = 0; i < cache->meteo_count; i++) {
        if (cache->meteo[i].id == cache->home[home_idx].id) {
            meteo_idx = i;
            break;
        }
    }
    
    if (meteo_idx < 0) {
        LOG_WARNING("No meteo data for home_id=%d", cache->home[home_idx].id);
        memset(solar_output, 0, sizeof(double) * 96);
        return -1;
    }
    
    Homesystem_t *home = &cache->home[home_idx];
    Meteo_t *meteo = &cache->meteo[meteo_idx];
    
    LOG_DEBUG("Predicting solar for home_id=%d (%.2f kWh capacity)", home->id, home->panel_capacitykwh);
    
    for (int slot = 0; slot < 96 && slot < KVARTAR_TOTALT; slot++) {
        // Extract hour and minute from timestamp to get time-of-day slot
        int hour, minute;
        sscanf(meteo->sample[slot].time_start, "%*d-%*d-%*dT%d:%d", &hour, &minute);
        int time_of_day_slot = (hour * 4) + (minute / 15);
        
        solar_output[slot] = solar_PredictSlot(
            meteo->sample[slot].ghi,
            meteo->sample[slot].cloud_cover,
            meteo->sample[slot].temp,
            home->panel_capacitykwh,
            home->panel_tiltdegrees,
            home->lat,
            time_of_day_slot,  // Use actual time-of-day, not array index!
            meteo->sample[slot].is_day
        );
    }

    return 0;
}

