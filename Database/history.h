#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>
#include <time.h>
#include <sqlite3.h>
#include "../Cache/CacheProtocol.h"

typedef struct {
    size_t meteo_rows;
    size_t spotpris_rows;
    char oldest_timestamp[32];
    char newest_timestamp[32];
} HistoryStats_t;

int history_Init(const char *db_path);
void history_Close(void);

int history_ArchiveMeteo(const Meteo_t *meteo, int id);
int history_ArchiveSpotpris(const Spot_t *spotpris, const char *area_name);

int history_GetStats(HistoryStats_t *stats);

#endif