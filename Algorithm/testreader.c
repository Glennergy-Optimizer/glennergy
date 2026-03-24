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
#include "utils.h"

//#define FIFO_ALGORITHM_READ "/tmp/fifo_algoritm"

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
            notify_fd = open(NOTIFY_FIFO_PATH, O_RDONLY | O_NONBLOCK);
            if (notify_fd < 0) {
                if (errno == ENXIO || errno == ENOENT) {
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

int test_reader()
{

    CacheData_t *cache = malloc(sizeof(CacheData_t));
    if (!cache) {
        LOG_ERROR("malloc() Failed to allocate memory for CacheData");
        return -1;
    }

    while(!SignalHandler_Stop())
    {
        if (algorithm_WaitForNotification() < 0) {
            LOG_INFO("Received shutdown signal, exiting...");
            break;
        }

        if (cacheRequest(CMD_GET_ALL, cache, sizeof(CacheData_t)) < 0) {
            LOG_WARNING("cacheRequest failed");
            continue;
        }
        LOG_INFO("Received from cache Meteo: %zu HomeSystem: %zu Spotpris: SE1=%zu SE2=%zu SE3=%zu SE4=%zu", cache->meteo_count, cache->home_count, cache->spotpris.count[0], cache->spotpris.count[1], cache->spotpris.count[2], cache->spotpris.count[3]);
        

        int spotpris_offset = -1;

        for (size_t j = 0; j < cache->meteo_count; j++)
        {
            spotpris_offset = calculateMeteoOffset(&cache->meteo[j], &cache->spotpris, 0);
            if (spotpris_offset >= 0) {
            LOG_INFO("Using meteo[%zu] start=%s, calculated spotpris offset=%d", j, cache->meteo[j].sample[0].time_start, spotpris_offset);
            break;
            }
        }

        if (spotpris_offset < 0) {
            LOG_WARNING("No valid meteo data available - cannot calculate offset");
            continue;
        }

        SpotStats_t spotpris_stats;
        if (average_SpotprisStats(&spotpris_stats, cache, spotpris_offset) < 0) {
            LOG_WARNING("Failed to calculate spotpris stats");
            continue;
        }
        
        ResultRequest algo_results = {0};
        algo_results.count = cache->home_count;

        for (size_t i = 0; i < cache->home_count; i++)
        {
            int area_idx = getAreaIndex(cache->home[i].electricity_area);
            if (area_idx < 0 || i >= cache->meteo_count) {
                LOG_WARNING("Invalid area or missing meteo for home_id=%d", cache->home[i].id);
                algo_results.results[i].valid = false;
                continue;
            }
            
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
        
        if (cacheSendResults(&algo_results) < 0) {
            LOG_ERROR("Failed to send results to cache");
        } else {
            LOG_INFO("Results sent to cache successfully");
        }
        
    }   //while
    LOG_INFO("Cleaning up and exiting...");
    free(cache);
    return 0;
}