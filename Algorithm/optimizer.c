#define MODULE_NAME "OPTIMIZER"
#include "../Server/Log/Logger.h"
#include "optimizer.h"
#include "average.h"
#include <string.h>
#include <time.h>

#define SOLAR_KWH_THRESHOLD 0.05 // Minimum solar output to consider it usable (kWh per 15-min slot)

// Get area index for home (called once per home)
static int get_area_index(const char *area)
{
    if (strcmp(area, "SE1") == 0) return 0;
    if (strcmp(area, "SE2") == 0) return 1;
    if (strcmp(area, "SE3") == 0) return 2;
    if (strcmp(area, "SE4") == 0) return 3;
    return -1;
}

int optimize_HomeEnergy(CacheData_t *cache, int home_idx, double *solar_predictions, SpotStats_t *spot_stats, AlgorithmResults_t *result)
{
    if (!cache || !solar_predictions || !spot_stats || !result) {
        LOG_ERROR("Invalid parameters");
        return -1;
    }
    
    if (home_idx < 0 || home_idx >= (int)cache->home_count) {
        LOG_ERROR("Invalid home index: %d", home_idx);
        return -1;
    }
    
    Homesystem_t *home = &cache->home[home_idx];
    
    // Get area index ONCE
    int area_idx = get_area_index(home->electricity_area);
    if (area_idx < 0) {
        LOG_ERROR("Invalid electricity area for home %d: %s", home->id, home->electricity_area);
        return -1;
    }
    
    // Initialize result
    memset(result, 0, sizeof(AlgorithmResults_t));
    result->home_id = home->id;
    result->last_calculated = time(NULL);
    result->valid = true;
    
    // Use pre-calculated q25 as cheap threshold
    double cheap_threshold = spot_stats->area[area_idx].q25;
    
    LOG_DEBUG("Home %d (area %s): using q25=%.4f SEK/kWh as cheap threshold", home->id, home->electricity_area, cheap_threshold);
    
    // Initialize tracking variables
    result->total_solar_kwh = 0.0;
    result->avg_grid_price = spot_stats->area[area_idx].average;
    result->peak_solar_slot = -1;
    result->cheapest_grid_slot = -1;
    result->most_expensive_slot = -1;
    
    double max_solar = 0.0;
    double min_price = 999.0;
    double max_price = 0.0;
    
    // Process each time slot
    for (int slot = 0; slot < 96; slot++) {
        double solar_kwh = solar_predictions[slot];
        
        double grid_price = 0.0;
        if (slot < (int)cache->spotpris.count[area_idx]) {
            grid_price = cache->spotpris.data[area_idx][slot].sek_per_kwh;
        }
        
        if (slot < (int)cache->spotpris.count[area_idx]) {
            strncpy(result->slots[slot].timestamp, cache->spotpris.data[area_idx][slot].time_start, sizeof(result->slots[slot].timestamp) - 1);
            result->slots[slot].timestamp[sizeof(result->slots[slot].timestamp) - 1] = '\0';
        } else {
            result->slots[slot].timestamp[0] = '\0';  // Empty if no data
        }
        result->slots[slot].solar_kwh = solar_kwh;
        result->slots[slot].grid_price = grid_price;
        
        // Determine strategy
        if (solar_kwh > SOLAR_KWH_THRESHOLD) {
            result->slots[slot].strategy = STRATEGY_USE_SOLAR;
        } else if (grid_price < cheap_threshold) {
            result->slots[slot].strategy = STRATEGY_USE_GRID_CHEAP;
        } else {
            result->slots[slot].strategy = STRATEGY_AVOID_GRID;
        }
        
        // Update summary stats
        result->total_solar_kwh += solar_kwh;
        
        // Track peaks
        if (solar_kwh > max_solar) {
            max_solar = solar_kwh;
            result->peak_solar_slot = slot;
        }
        
        if (grid_price > 0 && grid_price < min_price) {
            min_price = grid_price;
            result->cheapest_grid_slot = slot;
        }
        
        if (grid_price > max_price) {
            max_price = grid_price;
            result->most_expensive_slot = slot;
        }
    }
    
    LOG_INFO("Home %d: %.2f kWh solar/day, peak slot %d, cheapest grid slot %d, avoid slot %d",
             home->id,
             result->total_solar_kwh,
             result->peak_solar_slot, 
             result->cheapest_grid_slot,
             result->most_expensive_slot);
    
    return 0;
}