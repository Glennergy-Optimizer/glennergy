#ifndef CACHE_PROTOCOL_H
#define CACHE_PROTOCOL_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stddef.h>
#include "../Libs/Homesystem.h"

#define CACHE_SOCKET_PATH "/tmp/glennergy_cache.sock"
#define NOTIFY_FIFO_PATH "/tmp/fifo_notify_algorithm"

#define MAX_RESULTS 5
//#define NAME_MAX 128
#define KVARTAR_TOTALT 128
#define MAX_SLOTS 96

// ============================================
//  PROTOCOL COMMANDS & MESSAGES
// ============================================

typedef enum {
    NOTIFY_DATA_READY = 1,
    NOTIFY_SHUTDOWN = 9
} NotifyMessageType;

typedef struct {
    uint8_t type;           // NotifyMessageType
    uint8_t priority;       // Reserved for future use
    uint16_t data_count;    // For logging/debugging
    uint32_t timestamp;     // Unix timestamp
} __attribute__((packed)) NotifyMessage;

typedef enum {
    CMD_GET_ALL = 1,
    CMD_GET_METEO = 2,
    CMD_GET_SPOTPRIS = 3,
    CMD_SET_RESULT = 4,
    CMD_PING = 99
} CacheCommand;

typedef struct {
    uint8_t command;
    uint8_t reserved[3];
} CacheRequest;

typedef struct {
    uint32_t status;        // 0=success, 1=error
    uint32_t data_size;     //4,294,967,295 LETS GO
} CacheResponse;

// ============================================
//  ALGORITHM RESULTS
// ============================================

typedef enum {
    STRATEGY_USE_SOLAR = 0,
    STRATEGY_USE_GRID_CHEAP = 1,
    STRATEGY_AVOID_GRID = 2,
    STRATEGY_EXCESS_SOLAR = 3,
    STRATEGY_NO_DATA = 9
} EnergyStrategy_t;

typedef struct {
    char timestamp[32];
    double solar_kwh;
    double grid_price;
    EnergyStrategy_t strategy;
    double score;
} TimeSlot_t;

typedef struct {
    int home_id;
    time_t last_calculated;
    
    TimeSlot_t slots[MAX_SLOTS];

    double total_solar_kwh;
    double avg_grid_price;
    int peak_solar_slot;
    int cheapest_grid_slot;
    int most_expensive_slot;
    
    bool valid;
} AlgorithmResults_t;

typedef struct {
    AlgorithmResults_t results[MAX_RESULTS];
    size_t count;
} ResultRequest;

// ============================================
//  HOME SYSTEM STRUCTURES
// ============================================

// typedef struct {
//     int id;
//     char city[NAME_MAX];
//     double panel_capacitykw;
//     double panel_tiltdegrees;
//     double panel_azimuthdegrees;
//     double lat;
//     double lon;
//     char electricity_area[4];  // "SE1", "SE2", "SE3", "SE4"
// } Homesystem_t;

//======================================
//      METEO STRUCTURES
//======================================

typedef struct {
    char time_start[32];
    float temp;
    float ghi;
    float dni;
    float diffuse_radiation;
    float cloud_cover;
    int is_day;
    bool valid;
} MeteoEntry_t; 

typedef struct {
    int id;
    char city[NAME_MAX];
    double lat;
    double lon;
    MeteoEntry_t sample[KVARTAR_TOTALT];
} Meteo_t;

//======================================
//      SPOTPRIS STRUCTURES
//======================================

typedef enum {
    AREA_SE1 = 0,
    AREA_SE2 = 1,
    AREA_SE3 = 2,
    AREA_SE4 = 3,
    AREA_COUNT = 4
} SpotprisArea;

typedef struct {
    char time_start[32];
    double sek_per_kwh;
    //add count?
} SpotEntry_t;

typedef struct {
    SpotEntry_t data[AREA_COUNT][MAX_SLOTS];
    size_t count[AREA_COUNT];
} Spot_t;

// ============================================
//  CACHE DATA TRANSFER OBJECT
// ============================================

typedef struct CacheData_t {
    Homesystem_t home[MAX_RESULTS];
    size_t home_count;

    Meteo_t meteo[MAX_RESULTS];
    size_t meteo_count;
    Spot_t spotpris;
    // Don't expose internal home/shm!
} CacheData_t;


#endif