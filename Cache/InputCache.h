/**
 * @file InputCache.h
 * @brief Central cache module for Meteo and Spotpris data.
 *
 * @details
 * This module:
 * - Receives data from upstream producers (Meteo, Spotpris) via FIFO
 * - Stores latest state in memory
 * - Persists data to disk
 * - Serves data to clients via UNIX domain socket
 *
 * Acts as a central aggregation and distribution layer.
 *
 * @defgroup INPUTCACHE Input Cache
 * @ingroup CACHE
 * @{
 *
 * @note All data structures are stored in memory and ABI-sensitive
 * @note FIFO paths and UNIX socket paths are fixed and must match producer/consumer
 * @warning Access to shared data must be synchronized if accessed concurrently
 */

#ifndef INPUTCACHE_H
#define INPUTCACHE_H

#include "../API/Meteo/Meteo.h"
#include "../API/Spotpris/Spotpris.h"
#include "../Libs/Homesystem.h"
#include "../Libs/GlennergyPaths.h"

/** @brief FIFO path for Meteo input */
#define FIFO_METEO_READ GLENNERGY_METEO_FIFO_PATH

/** @brief FIFO path for Spotpris input */
#define FIFO_SPOTPRIS_READ GLENNERGY_SPOTPRIS_FIFO_PATH

/** @brief UNIX socket path for cache service */
#define CACHE_SOCKET_PATH GLENNERGY_CACHE_SOCKET_PATH

/** @brief Maximum socket backlog */
#define MAX_BACKLOG 5

/** @brief Maximum number of homes */
#define MAX_HOMES 5

/** @brief Maximum number of meteo entries */
#define MAX_METEO 5

/**
 * @brief Electricity price areas.
 *
 * @note Used for indexing spot price data arrays.
 */
typedef enum {
    AREA_SE1 = 0,
    AREA_SE2 = 1,
    AREA_SE3 = 2,
    AREA_SE4 = 3,
    AREA_COUNT = 4
} SpotprisArea;

/**
 * @brief Simplified Meteo data used by cache.
 *
 * @note Contains static array of weather samples.
 * @note All string fields are null-terminated.
 */
typedef struct {
    int id;                             /**< Property ID */
    char city[NAME_MAX];                /**< Property name */
    double lat;                         /**< Latitude */
    double lon;                         /**< Longitude */
    char electricity_area[5];           /**< Electricity area */
    Samples sample[KVARTAR_TOTALT];     /**< Weather samples */
} Meteo_t;

/**
 * @brief Single spot price entry.
 *
 * @note All string fields are null-terminated.
 */
typedef struct {
    char time_start[32];        /**< Timestamp */
    double sek_per_kwh;         /**< Price in SEK */
} SpotEntry_t;

/**
 * @brief Spot price container for all areas.
 *
 * @note 192 entries per area (48h * 4 quarters per hour)
 */
typedef struct {
    SpotEntry_t data[AREA_COUNT][192]; /**< 192 = 48h * 4 */
    size_t count[AREA_COUNT];          /**< Entries per area */
} Spot_t;

/**
 * @brief Main cache container.
 *
 * @note Holds Meteo, Spotpris, and Homesystem data.
 * @note `is_old` indicates stale data.
 */
typedef struct {
    Homesystem_t home[MAX_HOMES]; /**< Home configurations */
    size_t home_count;            /**< Number of homes */

    Meteo_t meteo[MAX_METEO];     /**< Meteo data */
    size_t meteo_count;           /**< Number of meteo entries */

    Spot_t spotpris;              /**< Spot price data */

    bool is_old;                  /**< Indicates stale data */
} InputCache_t;

/**
 * @brief Initialize cache and load home configuration.
 *
 * @param[out] cache Cache instance
 * @param[in] file_path Path to homesystem config
 *
 * @return 0 on success, -1 on failure
 *
 * @pre cache != NULL
 * @pre file_path != NULL
 * @post Cache initialized and home data loaded
 */
int inputcache_Init(InputCache_t *cache, const char* file_path);

/**
 * @brief Create and initialize UNIX socket server.
 *
 * @return Socket file descriptor, or -1 on failure
 *
 * @post Socket bound and listening
 */
int inputcache_CreateSocket(void);

/**
 * @brief Open FIFO channels for Meteo and Spotpris.
 *
 * @param[out] meteo_fd File descriptor for Meteo FIFO
 * @param[out] spotpris_fd File descriptor for Spotpris FIFO
 *
 * @return 0 on success, -1 on failure
 *
 * @pre meteo_fd != NULL
 * @pre spotpris_fd != NULL
 * @post FIFOs opened for reading
 */
int inputcache_OpenFIFOs(int *meteo_fd, int *spotpris_fd);

/**
 * @brief Handle incoming client request over socket.
 *
 * @param[in,out] cache Cache instance
 * @param[in] client_fd Connected client socket
 *
 * @post Client connection closed after handling
 */
void inputcache_HandleRequest(InputCache_t *cache, int client_fd);

/**
 * @brief Handle incoming Meteo data from FIFO.
 *
 * @param[in,out] cache Cache instance
 * @param[in] meteo_fd FIFO descriptor
 *
 * @post Cache Meteo data updated
 */
void inputcache_HandleMeteoData(InputCache_t *cache, int meteo_fd);

/**
 * @brief Handle incoming Spotpris data from FIFO.
 *
 * @param[in,out] cache Cache instance
 * @param[in] spotpris_fd FIFO descriptor
 *
 * @post Cache Spotpris data updated
 */
void inputcache_HandleSpotprisData(InputCache_t *cache, int spotpris_fd);

/**
 * @brief Cleanup cache resources.
 *
 * @param[in,out] cache Cache instance
 *
 * @post All allocated memory freed
 */
void inputcache_Cleanup(InputCache_t *cache);

/** @} */

#endif
