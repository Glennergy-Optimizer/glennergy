#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct Cache {
    char *cache_dir;
    time_t ttl;     // TTL in seconds
    pthread_mutex_t mutex;
} Cache;

int cache_Init(Cache *cache, const char *cache_dir, time_t set_ttl);

bool cache_Get(Cache *cache, const char *key, char **data, size_t *size);
int cache_Set(Cache *cache, const char *key, const char *data, size_t size);

bool cache_IsValid(Cache *cache, const char *key);

void cache_Dispose(Cache *cache);


#endif // CACHE_H