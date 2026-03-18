#ifndef CACHE_PROTOCOL_H
#define CACHE_PROTOCOL_H

#include <stdint.h>

#define CACHE_SOCKET_PATH "/tmp/glennergy_cache.sock"

typedef enum {
    CMD_GET_ALL = 1,
    CMD_GET_METEO = 2,
    CMD_GET_SPOTPRIS = 3,
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

#endif