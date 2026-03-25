#define MODULE_NAME "ALGORITM"
#include "../Server/Log/Logger.h"
#include "AlgoritmProtocol.h"
#include "../Libs/SHM.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "average.h"
#include <errno.h>

#include "../Cache/InputCache.h"
#include "../Cache/CacheProtocol.h"
#include "../Libs/Sockets.h"

#define FIFO_ALGORITHM_READ "/tmp/fifo_algoritm"

// gcc -Wall -Wextra -std=c11 -g testreader.c ../Sockets.c ../../Server/Log/Logger.c -I../../ -o testreader
int cache_request(CacheCommand cmd, void *data_out, size_t expected_size)
{
    if (!data_out || expected_size == 0)
    {
        LOG_ERROR("Invalid parameters for cache_request");
        return -1;
    }

    // Connect to cache socket
    int sock_fd = socket_Connect(CACHE_SOCKET_PATH);
    if (sock_fd < 0)
    {
        LOG_ERROR("Failed to connect to cache socket: %s", strerror(errno));
        return -1;
    }

    CacheResponse resp;

    // Send request
    CacheRequest req = {.command = cmd};
    if (send(sock_fd, &req, sizeof(req), 0) != sizeof(req))
    {
        LOG_ERROR("Failed to send request");
        close(sock_fd);
        return -1;
    }

    // Receive response header
    ssize_t bytes_read = recv(sock_fd, &resp, sizeof(resp), 0);
    if (bytes_read != sizeof(resp))
    {
        LOG_ERROR("Failed to receive response (got %zd bytes)", bytes_read);
        close(sock_fd);
        return -1;
    }

    if (resp.status != 0)
    {
        LOG_ERROR("Cache returned error status: %u", resp.status);
        close(sock_fd);
        return -1;
    }

    // Receive actual data
    bytes_read = recv(sock_fd, data_out, expected_size, 0);
    close(sock_fd);

    if (bytes_read != (ssize_t)expected_size)
    {
        LOG_ERROR("Failed to read complete data (got %zd, expected %zu bytes)",
                  bytes_read, expected_size);
        return -1;
    }

    return 0;
}

int main()
{
    log_Init("algoritm.log");

    InputCache_t *cache = malloc(sizeof(InputCache_t));
    if (!cache)
    {
        LOG_ERROR("malloc() Failed to allocate memory for InputCache");
        return -1;
    }

    AlgoritmShared *shm;
    int shm_fd = -1;
    sem_t *mutex;

    if (SHM_InitializeWriter(&shm, ALGORITM_SHARED, shm_fd) != 0)
    {
        return -1;
    }

    if (SHM_CreateSemaphore(&mutex, ALGORITM_MUTEX) != 0)
    {
        return -2;
    }

    memset(cache, 0, sizeof(InputCache_t));

    while (1)
    {
        if (cache_request(CMD_GET_ALL, cache, sizeof(InputCache_t)) < 0)
        {
            LOG_ERROR("Failed to get data from cache, retrying in 5 seconds...");
            sleep(5);
        }

        // LOG_INFO("Received from cache Meteo: %zu HomeSystem: %zu price areas: %zu", cache->meteo_count, cache->home_count, sizeof(cache->spotpris.count) / sizeof(cache->spotpris.count[0]));

        const char *area_names[AREA_COUNT] = {"SE1", "SE2", "SE3", "SE4"};

        size_t spot_index = 0;

        SpotStats_t stats;
        average_SpotprisStats(&stats, cache);

        sem_wait(mutex);
        for (size_t area_idx = 0; area_idx < 4; area_idx++)
        {

            size_t show_count = cache->spotpris.count[area_idx];
            // if (show_count > 96)
            // show_count = 96; // Show only first 10

            for (size_t entry = 0; entry < show_count; entry++)
            {
                if (strncmp(cache->meteo[0].sample[0].time_start, cache->spotpris.data[area_idx][entry].time_start, 16) == 0)
                {
                    spot_index = entry; // Get the active index for spotpris
                }
            }

            size_t spot_iterator = (spot_index + 96); // Add 96 quarters to get accurate matched price 24 hrs forward

            if (spot_iterator > cache->spotpris.count[area_idx])
            {
                spot_iterator = cache->spotpris.count[area_idx];
            }

            for (size_t i = 0; i < cache->meteo_count; i++)
            {
                printf("before shared: %s\n", cache->meteo[i].electricity_area);
                printf("Comparing meteo area '%s' with spotpris area '%s'\n", cache->meteo[i].electricity_area, area_names[area_idx]);
                if (strncmp(cache->meteo[i].electricity_area, area_names[area_idx], 3) == 0)
                {
                    for (size_t entry = spot_index; entry < spot_iterator; entry++)
                    {
                        printf("Area: %s, time: %s, price: %.2f SEK/kWh,\n",
                               area_names[area_idx],
                               cache->spotpris.data[area_idx][entry].time_start,
                               cache->spotpris.data[area_idx][entry].sek_per_kwh);

                        for (size_t j = 0; j < 96; j++)
                        {
                            if (strncmp(cache->meteo[i].sample[j].time_start, cache->spotpris.data[area_idx][entry].time_start, 16) == 0)
                            {
                                shm->result[i].id = cache->meteo[i].id;
                                int temp = average_WindowLow_test(&cache->spotpris.data[area_idx][entry], stats.area[area_idx].q25, stats.area[area_idx].q75);

                                if (temp > 0)
                                {
                                    shm->result[i].recommendation[j] = temp;
                                    snprintf(shm->result[i].time[j].time, sizeof(shm->result[i].time[j].time), "%s", cache->spotpris.data[area_idx][entry].time_start);
                                }

                                printf("  Matched time: %s, temp: %.2f °C, GHI: %.2f W/m², City: %s id: %d\n",
                                       cache->meteo[i].sample[j].time_start,
                                       cache->meteo[i].sample[j].temp,
                                       cache->meteo[i].sample[j].ghi,
                                       cache->meteo[i].city,
                                       cache->meteo[i].id);
                            }
                        }
                    }
                }
            }
        }
        sem_post(mutex);
        sleep(10);
    }
    printf("Free cache\n");
    SHM_CloseSemaphore(&mutex);
    SHM_DisposeWriter(&shm, ALGORITM_SHARED, shm_fd);
    free(cache);
    log_Cleanup();
    return 0;
}