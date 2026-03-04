#ifndef INPUTCACHE_H
#define INPUTCACHE_H


#include "../API/Meteo/Meteo.h"
#include "../API/Spotpris/Spotpris.h"
#include "../Libs/Homesystem.h"
#include "../Libs/Shm.h"

#define FIFO_METEO_READ "/tmp/fifo_meteo"
#define FIFO_SPOTPRIS_READ "/tmp/fifo_spotpris"
#define CACHE_SOCKET_PATH "/tmp/glennergy_cache.sock"

#define MAX_BACKLOG 5
#define MAX 5

typedef enum {
    AREA_SE1 = 0,
    AREA_SE2 = 1,
    AREA_SE3 = 2,
    AREA_SE4 = 3,
    AREA_COUNT = 4
} SpotprisArea;

typedef struct {
    int id;
    char city[NAME_MAX];
    double lat;
    double lon;
    Samples sample[KVARTAR_TOTALT];
} Meteo_t;

typedef struct {
    char time_start[32];
    double sek_per_kwh;
} SpotEntry_t;

typedef struct {
    SpotEntry_t data[AREA_COUNT][96];
    size_t count[AREA_COUNT];
} Spot_t;

typedef struct {

    Homesystem_t home[MAX];
    size_t home_count;

    Meteo_t meteo[MAX];
    size_t meteo_count;

    Spot_t spotpris; // Spot_t spotpris[AREA_MAX][SAMPLES]

    bool updated_meteo;
    bool updated_spotpris;

    SharedData_t *shm;
} InputCache_t;

typedef struct {
    InputCache_t *cache;

    int meteo_fd;
    int spotpris_fd;
    int socket_fd;
} InputCacheContext_t;

int inputcache_InitAll(InputCacheContext_t *ctx, const char* file_path);

void inputcache_HandleRequest(InputCache_t *cache, int client_fd);
void inputcache_HandleMeteoData(InputCache_t *cache, int meteo_fd);
void inputcache_HandleSpotprisData(InputCache_t *cache, int spotpris_fd);

void inputcache_SendNotification(NotifyMessageType type, uint16_t count);

void inputcache_CleanupAll(InputCacheContext_t *ctx);
#endif