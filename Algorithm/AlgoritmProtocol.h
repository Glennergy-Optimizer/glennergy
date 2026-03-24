#ifndef TESTREADER_H
#define TESTREADER_H

#define MAX_ID 5

#define ALGORITM_SHARED "/algoritm_shm"
#define ALGORITM_MUTEX "/algoritm_mutex"

int test_reader();

typedef struct
{
    char time[32];
} time_start;

typedef struct
{
    int id;
    double recommendation[96];
    time_start time[96];
} AlgoritmResult;

typedef struct
{
    AlgoritmResult result[MAX_ID];
} AlgoritmShared;

#endif