#define MODULE_NAME "TESTREADER"
#include "../Server/Log/Logger.h"
#include "testreader.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "average.h"
#include <errno.h>
#include "../Server/SignalHandler.h"
#include "../Cache/InputCache.h"
#include "../Cache/CacheProtocol.h"
#include "../Libs/Sockets.h"

#define FIFO_ALGORITHM_READ "/tmp/fifo_algoritm"

int algorithm_WaitForNotification(void)
{
    static int notify_fd = -1;
    NotifyMessage msg;
    
    // Open on first call
    if (notify_fd < 0) {
        notify_fd = open(NOTIFY_FIFO_PATH, O_RDONLY);
        if (notify_fd < 0) {
            LOG_ERROR("Failed to open notification pipe: %s", strerror(errno));
            return -1;
        }
        LOG_INFO("Notification pipe opened, waiting for events...");
    }
    
    ssize_t bytes_read = read(notify_fd, &msg, sizeof(NotifyMessage));
    
    if (bytes_read == sizeof(NotifyMessage)) {
        if (msg.type == NOTIFY_SHUTDOWN) {
            LOG_INFO("Shutdown notification received");
            close(notify_fd);
            notify_fd = -1;
            return -2;  // Special return code for shutdown
        }
        LOG_DEBUG("Received notification (type=%d, count=%u)", msg.type, msg.data_count);
        return 0;  // Success - data ready

    } else if (bytes_read == 0) {
        LOG_WARNING("Notification pipe closed (InputCache shutdown?)");
        close(notify_fd);
        notify_fd = -1;
        return -1;  // EOF
    } else if (bytes_read > 0) {
        LOG_ERROR("Partial message received (%zd bytes, expected %zu)", bytes_read, sizeof(NotifyMessage));
        return -1;
    } else {
        LOG_ERROR("Read error: %s", strerror(errno));
        close(notify_fd);
        notify_fd = -1;
        return -1;
    }
}

int cache_request(CacheCommand cmd, void *data_out, size_t expected_size)
{
    if (!data_out || expected_size == 0) {
        LOG_ERROR("Invalid parameters for cache_request");
        return -1;
    }

    int sock_fd = socket_Connect(CACHE_SOCKET_PATH);
    if (sock_fd < 0) {
        LOG_ERROR("Failed to connect to cache socket: %s", strerror(errno));
        return -1;
    }

    CacheRequest req = { .command = cmd };
    if (send(sock_fd, &req, sizeof(req), 0) != sizeof(req)) {
        LOG_ERROR("Failed to send request");
        close(sock_fd);
        return -1;
    }

    CacheResponse resp;
    ssize_t bytes_read = recv(sock_fd, &resp, sizeof(resp), 0);
    if (bytes_read != sizeof(resp)) {
        LOG_ERROR("Failed to receive response (got %zd bytes)", bytes_read);
        close(sock_fd);
        return -1;
    }

    if (resp.status != 0) {
        LOG_ERROR("Cache returned error status: %u", resp.status);
        close(sock_fd);
        return -1;
    }

    bytes_read = recv(sock_fd, data_out, expected_size, 0);
    close(sock_fd);

    if (bytes_read != (ssize_t)expected_size) {
        LOG_ERROR("Failed to read complete data (got %zd, expected %zu bytes)", 
                  bytes_read, expected_size);
        return -1;
    }

    return 0;
}

int cache_SendResults(const ResultRequest *results)
{
    int sock_fd = socket_Connect(CACHE_SOCKET_PATH);
    if (sock_fd < 0) {
        LOG_ERROR("Failed to connect to cache");
        return -1;
    }

    CacheRequest req = { .command = CMD_SET_RESULT };
    send(sock_fd, &req, sizeof(req), 0);
    send(sock_fd, results, sizeof(ResultRequest), 0);

    CacheResponse resp;
    recv(sock_fd, &resp, sizeof(resp), 0);
    close(sock_fd);
    
    if (resp.status != 0) {
        LOG_ERROR("Cache rejected results");
        return -1;
    }

    LOG_INFO("Sent %zu results to cache", results->count);
    return 0;
}

int test_reader()
{

    InputCache_t *cache = malloc(sizeof(InputCache_t));
    if (!cache) {
        LOG_ERROR("malloc() Failed to allocate memory for InputCache");
        return -1;
    }

    while(!SignalHandler_Stop())
    {
        int notify_result = algorithm_WaitForNotification();
        if (notify_result == -2) {
            LOG_INFO("Received shutdown notification, exiting...");
            break;
        } else if (notify_result < 0) {
            LOG_ERROR("Error waiting for notification");
            sleep(10); // Wait before retrying
            continue;
        }

        if (cache_request(CMD_GET_ALL, cache, sizeof(InputCache_t)) < 0) {
            LOG_WARNING("cache_request failed");
            continue;
        }

        LOG_INFO("Received from cache Meteo: %zu HomeSystem: %zu Spotpris: SE1=%zu SE2=%zu SE3=%zu SE4=%zu", cache->meteo_count, cache->home_count, cache->spotpris.count[0], cache->spotpris.count[1], cache->spotpris.count[2], cache->spotpris.count[3]);

        SpotStats_t spotpris_stats;

        int result = average_SpotprisStats(&spotpris_stats, cache);
        int window_result = average_WindowLow(cache, spotpris_stats.area[0].q25);

        ResultRequest algo_results = {0};
        algo_results.count = cache->home_count;

        for (size_t i = 0; i < cache->home_count; i++)
        {
            double solar_predictions[96];
            if (solar_PredictHome(cache, i, solar_predictions) < 0) {
                LOG_WARNING("Failed to predict solar for home_id=%d", cache->home[i].id);
                algo_results.results[i].valid = false;
                continue;
            }

            if (optimize_HomeEnergy(cache, i, solar_predictions, &spotpris_stats, &algo_results.results[i]) < 0) {
                LOG_WARNING("Failed to optimize energy for home_id=%d", cache->home[i].id);
                algo_results.results[i].valid = false;
                continue;
            }

            LOG_INFO("Home: %d solar: %.2f kWh, peak slot: %d, cheapest grid slot: %d, most expensive slot: %d",
                    algo_results.results[i].home_id,
                    algo_results.results[i].total_solar_kwh,
                    algo_results.results[i].peak_solar_slot,
                    algo_results.results[i].cheapest_grid_slot,
                    algo_results.results[i].most_expensive_slot);

        }
        if (cache_SendResults(&algo_results) < 0) {
            LOG_ERROR("Failed to send results to cache");
        } else {
            LOG_INFO("Results sent to cache successfully");
        }
        
    }   //while
    LOG_INFO("Cleaning up and exiting...");
    free(cache);
    return 0;
}