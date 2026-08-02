/**
 * @file main.c
 * @brief Entry point for the Algorithm module.
 *
 * @ingroup Algorithm
 *
 * Reads cached data, computes recommendations, and publishes the result to
 * shared memory.
 */

#define MODULE_NAME "ALGORITM"

#include "../Server/Log/Logger.h"
#include "../Server/SignalHandler.h"
#include "AlgoritmProtocol.h"
#include "../Libs/SHM.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "average.h"
#include <errno.h>
#include <stdbool.h>

#include "../Cache/InputCache.h"
#include "../Cache/CacheProtocol.h"
#include "../Libs/Sockets.h"

/**
 * @brief Sends a request to the cache and receives the expected data.
 *
 * @param cmd Cache command to send.
 * @param data_out Pointer to memory where data is stored.
 * @param expected_size Expected size of the output data.
 *
 * @return 0 on success, -1 on error.
 *
 * @warning `data_out` must point to valid, writable memory.
 * @note Logs errors using `LOG_ERROR` and closes the socket on failure.
 */
// gcc -Wall -Wextra -std=c11 -g testreader.c ../Sockets.c ../../Server/Log/Logger.c -I../../ -o testreader


static ssize_t recv_all(int fd, void *buf, size_t size)
{
    size_t total = 0;
    char *buffer = buf;

    while (total < size)
    {
        ssize_t bytes = recv(fd, buffer + total, size - total, 0);

        if (bytes == 0)
        {
            break;
        }

        if (bytes < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return -1;
        }

        total += bytes;
    }

    return (ssize_t)total;
}

/**
 * @brief Sends a cache request and reads the response payload.
 *
 * @param cmd Cache command to send.
 * @param data_out Destination buffer for the payload.
 * @param expected_size Expected payload size in bytes.
 *
 * @return 0 on success, -1 on error.
 */
int cache_request(CacheCommand cmd, void *data_out, size_t expected_size)
{
    if (!data_out || expected_size == 0)
    {
        LOG_ERROR("Invalid parameters for cache_request");
        return -1;
    }

    // Connect to cache socket
    int sock_fd = socket_Connect(GLENNERGY_CACHE_SOCKET_PATH);
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
    ssize_t bytes_read = recv_all(sock_fd, &resp, sizeof(resp));
    if (bytes_read != sizeof(resp))
    {
        LOG_ERROR("Payload size mismatch: cache says %u, expected %zu", resp.data_size, expected_size);
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
    bytes_read = recv_all(sock_fd, data_out, expected_size);
    close(sock_fd);

    if (bytes_read != (ssize_t)expected_size)
    {
        LOG_ERROR("Failed to read complete data (got %zd, expected %zu bytes)",
                  bytes_read, expected_size);
        return -1;
    }

    return 0;
}



/**
 * @brief Main loop for the Algorithm module.
 *
 * Initializes logging, shared memory, semaphores, and InputCache,
 * then enters a loop to:
 * - Fetch data from cache
 * - Compute spot price statistics
 * - Match meteo and spot prices
 * - Store recommendations in shared memory
 *
 * @return 0 on normal exit, -1/-2 on initialization errors
 *
 * @warning Runs an infinite loop with sleep(10); ensure proper termination in production.
 * @note Frees allocated InputCache memory and cleans up logging before exit.
 */
int main()
{
    log_Init("algoritm.log");
    SignalHandler_Initialize();

    InputCache_t *cache = malloc(sizeof(InputCache_t));
    if (!cache)
    {
        LOG_ERROR("malloc() Failed to allocate memory for InputCache");
        return -1;
    }

    AlgoritmShared *shm;
    //int shm_fd = -1;
    sem_t *mutex;

    //if (SHM_InitializeWriter(&shm, ALGORITM_SHARED, shm_fd) != 0)
    if (SHM_InitializeWriter(&shm, ALGORITM_SHARED) != 0)
    {
        free(cache); //Free our earlier malloc
        log_Cleanup(); // Close logging 
        return -1;
    }

    if (SHM_CreateSemaphore(&mutex, ALGORITM_MUTEX) != 0)
    {
        SHM_DisposeWriter(&shm); //Unmap the shared memory if failed to create a semaphore
        free(cache); 
        log_Cleanup();
        return -2;
    }

    memset(cache, 0, sizeof(InputCache_t));
    while (!SignalHandler_Stop())
    {
        if (cache_request(CMD_GET_ALL, cache, sizeof(InputCache_t)) < 0)
        {
            LOG_ERROR("Failed to get data from cache, retrying in 5 seconds...");
            sleep(5);
            // If something fails, don't publish, just continue
            continue;
        }

        // LOG_INFO("Received from cache Meteo: %zu HomeSystem: %zu price areas: %zu", cache->meteo_count, cache->home_count, sizeof(cache->spotpris.count) / sizeof(cache->spotpris.count[0]));

        const char *area_names[AREA_COUNT] = {"SE1", "SE2", "SE3", "SE4"};

        // Create a new snapshot every cycle and memset so old recommendation values doesnt survive
        AlgoritmShared next_shm;
        memset(&next_shm, 0, sizeof(next_shm));

        for (size_t area_idx = 0; area_idx < 4; area_idx++)
        {
            size_t show_count = cache->spotpris.count[area_idx];
            // Now attempts to find the spotpris index of the first meteo timestamp
            size_t spot_index = 0;
            bool found_spot_index = false;
            // if (show_count > 96)
            // show_count = 96; // Show only first 10

            for (size_t entry = 0; entry < show_count; entry++)
            {
                if (strncmp(cache->meteo[0].sample[0].time_start, cache->spotpris.data[area_idx][entry].time_start, 16) == 0)
                {
                    spot_index = entry; // Get the active index for spotpris
                    found_spot_index = true;
                    break;
                }
            }

            // If not found, skip. TODO - Re-evaluate how we handle errors?
            if (!found_spot_index)
            {
                LOG_ERROR("No matching spot price start index found for area %s", area_names[area_idx]);
                continue;
            }

            size_t spot_iterator = (spot_index + 96); // Add 96 quarters to get accurate matched price 24 hrs forward

            if (spot_iterator > cache->spotpris.count[area_idx])
            {
                spot_iterator = cache->spotpris.count[area_idx];
            }

            Stats_t window_stats;
            if (average_SpotprisStatsRange(&window_stats, &cache->spotpris,
                                           area_idx, spot_index,
                                           spot_iterator) != 0)
            {
                LOG_ERROR("Invalid forward price window for area %s", area_names[area_idx]);
                continue;
            }

            for (size_t i = 0; i < cache->meteo_count; i++)
            {
                if (strncmp(cache->meteo[i].electricity_area, area_names[area_idx], 3) == 0)
                {
                    for (size_t entry = spot_index; entry < spot_iterator; entry++)
                    {
                        for (size_t j = 0; j < 96; j++)
                        {
                            if (strncmp(cache->meteo[i].sample[j].time_start, cache->spotpris.data[area_idx][entry].time_start, 16) == 0)
                            {
                                next_shm.result[i].id = cache->meteo[i].id;
                                double score = average_WindowLow_percent(&cache->spotpris.data[area_idx][entry], window_stats.min, window_stats.max);
                                int recommendation_type = average_WindowLow_test(&cache->spotpris.data[area_idx][entry], window_stats.q25, window_stats.q75);

                                next_shm.result[i].recommendation[j] = score;
                                next_shm.result[i].recommendation_type[j] = recommendation_type;
                                next_shm.result[i].weather.temp[j] = cache->meteo[i].sample[j].temp;
                                next_shm.result[i].weather.weather_code[j] = cache->meteo[i].sample[j].weather_code;
                                next_shm.result[i].weather.uv_index[j] = cache->meteo[i].sample[j].uv_index;
                                next_shm.result[i].price[j] = cache->spotpris.data[area_idx][entry].sek_per_kwh;

                                snprintf(next_shm.result[i].time[j].time, sizeof(next_shm.result[i].time[j].time), "%s", cache->spotpris.data[area_idx][entry].time_start);

                            }
                        }
                    }
                }
            }
        }

        //  publish completed snapshot for the readers
        if (SignalHandler_Stop())
            break;

        if (sem_wait(mutex) != 0)
        {
            if (errno == EINTR && SignalHandler_Stop())
                break;

            LOG_ERROR("Failed to lock algorithm shared memory: %s", strerror(errno));
            continue;
        }
        memcpy(shm, &next_shm, sizeof(next_shm));
        sem_post(mutex);
        sleep(10);
    }
    printf("Free cache\n");
    SHM_CloseSemaphore(&mutex);
    SHM_DisposeWriter(&shm);
    free(cache);
    log_Cleanup();
    return 0;
}
