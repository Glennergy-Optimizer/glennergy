#define MODULE_NAME "TESTREADER"
#include "../Server/Log/Logger.h"
#include "testreader.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include "average.h"
#include "solar.h"
#include "optimizer.h"
#include "../Server/SignalHandler.h"
#include "../Libs/Sockets.h"

#define FIFO_ALGORITHM_READ "/tmp/fifo_algoritm"

int algorithm_WaitForNotification(void)
{
    static int notify_fd = -1;
    NotifyMessage msg;
    
    while(1)
    {
        if (SignalHandler_Stop()) {
            if (notify_fd >= 0) {
                close(notify_fd);
                notify_fd = -1;
            }
            return -2;
        }

        if (notify_fd < 0) {
            notify_fd = open(NOTIFY_FIFO_PATH, O_RDONLY);
            if (notify_fd < 0) {
                if (errno == ENXIO) {
                    LOG_WARNING("No writers on notification pipe, retrying...");
                    sleep(1);
                    continue;
                }
                LOG_WARNING("Failed to open notification pipe: %s", strerror(errno));
                sleep(5);
                continue;
            }
            
            int flags = fcntl(notify_fd, F_GETFL, 0);
            fcntl(notify_fd, F_SETFL, flags & ~O_NONBLOCK);
            LOG_INFO("Notification pipe connected");
        }
        
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(notify_fd, &read_fds);
        struct timeval timeout = {1, 0};
        
        int ready = select(notify_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (ready < 0) {
            if (errno == EINTR) {
                continue;  // Check SignalHandler_Stop() at loop start
            }
            LOG_WARNING("select() error: %s", strerror(errno));
            close(notify_fd);
            notify_fd = -1;
            sleep(5);
            continue;
        }
        
        if (ready == 0)
            continue;

        ssize_t bytes_read = read(notify_fd, &msg, sizeof(NotifyMessage));
        
        if (bytes_read == sizeof(NotifyMessage)) {
            LOG_INFO("Received notification (type=%d, count=%u)", msg.type, msg.data_count);
            return 0;  // Success - data ready
        }
        
        if (bytes_read == 0) {
            LOG_INFO("inputcache disconnect, will reconnect...");
        } else {
            LOG_WARNING("Read error: %s", strerror(errno));
        }
        close(notify_fd);
        notify_fd = -1;
        sleep(5);
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

    CacheData_t *cache = malloc(sizeof(CacheData_t));
    if (!cache) {
        LOG_ERROR("malloc() Failed to allocate memory for CacheData");
        return -1;
    }

    while(!SignalHandler_Stop())
    {
    //     int notify_result = algorithm_WaitForNotification();
    //     if (notify_result == -2) {
    //         LOG_INFO("Received shutdown notification, exiting...");
    //         break;
    //    } 
    //    // else if (notify_result < 0) {
    //     //     LOG_ERROR("Error waiting for notification");
    //     //     sleep(10); // Wait before retrying
    //     //     continue;
    //     // }
        if (algorithm_WaitForNotification() < 0) {
            LOG_INFO("Received shutdown signal, exiting...");
            break;
        }

        if (cache_request(CMD_GET_ALL, cache, sizeof(CacheData_t)) < 0) {
            LOG_WARNING("cache_request failed");
            continue;
        }

        LOG_INFO("Received from cache Meteo: %zu HomeSystem: %zu Spotpris: SE1=%zu SE2=%zu SE3=%zu SE4=%zu", cache->meteo_count, cache->home_count, cache->spotpris.count[0], cache->spotpris.count[1], cache->spotpris.count[2], cache->spotpris.count[3]);

        SpotStats_t spotpris_stats;

        int result = average_SpotprisStats(&spotpris_stats, cache);
        if (result < 0) {
            LOG_WARNING("Failed to calculate spotpris stats");
        }
        int window_result = average_WindowLow(cache, spotpris_stats.area[0].q25);
        if (window_result < 0) {
            LOG_WARNING("Failed to calculate window low");
        }

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