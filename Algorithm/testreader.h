#ifndef TESTREADER_H
#define TESTREADER_H

#include <semaphore.h>
#include "../Cache/CacheProtocol.h"

#define MAX_ID 5

#define ALGORITM_SHARED "/algoritm_shm"
#define ALGORITM_MUTEX "/algoritm_mutex"


typedef struct
{
    char time[32];
} time_start;

typedef struct
{
    int id;
    int recommendation[96];
    time_start time[96];
} AlgoritmResult;

typedef struct
{
    AlgoritmResult result[MAX_ID];
} SharedMemory;

int test_reader(SharedMemory *shm, sem_t *mutex);

// #include "../Cache/CacheProtocol.h"
// int test_reader();
// int cache_request(CacheCommand cmd, void *data_out, size_t expected_size);
// int cache_SendResults(const ResultRequest *results);

#endif