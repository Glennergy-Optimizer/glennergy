#ifndef INPUTCACHE_H
#define INPUTCACHE_H

#include "CacheProtocol.h"
#include <stdbool.h>

typedef struct SharedData_t SharedData_t;

typedef struct {
    CacheData_t *cache;

    bool updated_meteo;
    bool updated_spotpris;
    SharedData_t *shm;

    int meteo_fd;
    int spotpris_fd;
    int socket_fd;
} InputCacheContext_t;

int inputcache_InitAll(InputCacheContext_t *ctx, const char* file_path);

void inputcache_HandleRequest(InputCacheContext_t *ctx, int client_fd);
void inputcache_HandleMeteoData(InputCacheContext_t *ctx, int meteo_fd);
void inputcache_HandleSpotprisData(InputCacheContext_t *ctx, int spotpris_fd);

void inputcache_SendNotification(NotifyMessageType type, uint16_t count);

void inputcache_CleanupAll(InputCacheContext_t *ctx);
#endif