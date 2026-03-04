#ifndef CACHE_PROTOCOL_H
#define CACHE_PROTOCOL_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stddef.h>

#define CACHE_SOCKET_PATH "/tmp/glennergy_cache.sock"
#define NOTIFY_FIFO_PATH "/tmp/fifo_notify_algorithm"

#define MAX_RESULTS 5

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

typedef enum {
    STRATEGY_USE_SOLAR = 0,
    STRATEGY_USE_GRID_CHEAP = 1,
    STRATEGY_AVOID_GRID = 2,
    STRATEGY_EXCESS_SOLAR = 3,
} EnergyStrategy_t;

typedef struct {
    double solar_kwh;
    double grid_price;
    EnergyStrategy_t strategy;
} TimeSlot_t;

typedef struct {
    int home_id;
    time_t last_calculated;
    
    TimeSlot_t slots[96];

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

#endif