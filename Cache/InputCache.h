/**
 * @file InputCache.h
 * @brief Public API for the InputCache module.
 *
 * Provides cache types and functions for Meteo and Spotpris data.
 *
 * @defgroup INPUTCACHE Input Cache
 * @ingroup CACHE
 * @{
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
 * Used for indexing spot price data arrays.
 */
typedef enum {
    AREA_SE1 = 0,
    AREA_SE2 = 1,
    AREA_SE3 = 2,
    AREA_SE4 = 3,
    AREA_COUNT = 4
} SpotprisArea;

/**
 * @brief Simplified Meteo data used by the cache.
 *
 * Contains the fixed-size weather data copied from the upstream Meteo input.
 */
typedef struct {
    int id;                             /**< Property ID. */
    char city[NAME_MAX];                /**< Property name. */
    double lat;                         /**< Latitude. */
    double lon;                         /**< Longitude. */
    char electricity_area[5];           /**< Electricity area. */
    Samples sample[KVARTAR_TOTALT];     /**< Weather samples. */
} Meteo_t;

/**
 * @brief Single spot price entry.
 */
typedef struct {
    char time_start[32];        /**< Timestamp. */
    double sek_per_kwh;         /**< Price in SEK. */
} SpotEntry_t;

/**
 * @brief Spot price container for all areas.
 *
 * Stores one fixed-size array per area together with the number of valid
 * entries in each array.
 */
typedef struct {
    SpotEntry_t data[AREA_COUNT][192]; /**< 192 = 48h * 4. */
    size_t count[AREA_COUNT];          /**< Entries per area. */
} Spot_t;

/**
 * @brief Main cache container.
 *
 * Holds homesystem, Meteo, and Spotpris data for serving clients.
 */
typedef struct {
    Homesystem_t home[MAX_HOMES]; /**< Home configurations. */
    size_t home_count;            /**< Number of homes. */

    Meteo_t meteo[MAX_METEO];     /**< Meteo data. */
    size_t meteo_count;           /**< Number of Meteo entries. */

    Spot_t spotpris;              /**< Spot price data. */

    bool is_old;                  /**< Indicates stale data. */
} InputCache_t;

/**
 * @brief Initializes the cache and loads home configuration.
 *
 * @param[out] cache Cache instance.
 * @param[in] file_path Path to the homesystem configuration file.
 *
 * @return 0 on success, -1 on failure.
 */
int inputcache_Init(InputCache_t *cache, const char* file_path);

/**
 * @brief Creates and initializes the UNIX socket server.
 *
 * @return Socket file descriptor, or -1 on failure.
 */
int inputcache_CreateSocket(void);

/**
 * @brief Opens FIFO channels for Meteo and Spotpris.
 *
 * @param[out] meteo_fd File descriptor for the Meteo FIFO.
 * @param[out] spotpris_fd File descriptor for the Spotpris FIFO.
 *
 * @return 0 on success, -1 on failure.
 */
int inputcache_OpenFIFOs(int *meteo_fd, int *spotpris_fd);

/**
 * @brief Handles an incoming client request over the socket.
 *
 * @param[in,out] cache Cache instance.
 * @param[in] client_fd Connected client socket.
 */
void inputcache_HandleRequest(InputCache_t *cache, int client_fd);

/**
 * @brief Handles incoming Meteo data from FIFO.
 *
 * @param[in,out] cache Cache instance.
 * @param[in] meteo_fd FIFO descriptor.
 */
void inputcache_HandleMeteoData(InputCache_t *cache, int meteo_fd);

/**
 * @brief Handles incoming Spotpris data from FIFO.
 *
 * @param[in,out] cache Cache instance.
 * @param[in] spotpris_fd FIFO descriptor.
 */
void inputcache_HandleSpotprisData(InputCache_t *cache, int spotpris_fd);

/**
 * @brief Cleans up cache resources.
 *
 * @param[in,out] cache Cache instance.
 */
void inputcache_Cleanup(InputCache_t *cache);

/** @} */

#endif
