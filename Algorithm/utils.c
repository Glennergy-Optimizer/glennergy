#define MODULE_NAME "UTILS"
#include "../Server/Log/Logger.h"
#include "utils.h"
#include "../Libs/Sockets.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int getAreaIndex(const char *area)
{
    if (strcmp(area, "SE1") == 0) return 0;
    if (strcmp(area, "SE2") == 0) return 1;
    if (strcmp(area, "SE3") == 0) return 2;
    if (strcmp(area, "SE4") == 0) return 3;
    return -1;
}

int calculateMeteoOffset(const Meteo_t *meteo, const Spot_t *spotpris, int area_idx)
{
    if (meteo->sample[0].time_start[0] == '\0' || spotpris->count[area_idx] == 0) {
        return -1;
    }
    
    // Find matching timestamp in spotpris array
    // Meteo: "2026-03-24T13:15Z", Spotpris: "2026-03-24T13:15:00Z"
    // Compare first 16 chars (up to minutes: "2026-03-24T13:15")
    for (int i = 0; i < spotpris->count[area_idx]; i++)
    {
        if (strncmp(meteo->sample[0].time_start, spotpris->data[area_idx][i].time_start, 16) == 0)
        {
            LOG_DEBUG("Meteo starts at %s, matched spotpris slot %d", meteo->sample[0].time_start, i);
            return i;
        }
    }
    
    LOG_WARNING("No matching spotpris slot for meteo time %s", meteo->sample[0].time_start);
    return 0;
}

int cacheRequest(CacheCommand cmd, void *data_out, size_t expected_size)
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

    size_t total_read = 0;
    while (total_read < expected_size) {
        bytes_read = recv(sock_fd, (char *)data_out + total_read, expected_size - total_read, 0);
        
        if (bytes_read <= 0) {
            LOG_ERROR("Socket error during read (got %zu/%zu bytes)", total_read, expected_size);
            close(sock_fd);
            return -1;
        }
        total_read += bytes_read;
    }
    close(sock_fd);

    LOG_DEBUG("Successfully received %zu bytes", total_read);
    return 0;
}

int cacheSendResults(const ResultRequest *results)
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
    return (resp.status == 0) ? 0 : -1;
}