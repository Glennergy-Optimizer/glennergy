#define MODULE_NAME "MAIN"
#include "../Server/Log/Logger.h"
#include "InputCache.h"
#include "CacheProtocol.h"
#include "../Libs/Pipes.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include "../Server/SignalHandler.h"

int main()
{
    int ret = -1;
    log_Init("cache.log");
    SignalHandler_Initialize();
    LOG_INFO("Starting Cache module...");

    InputCacheContext_t ctx;

    if (inputcache_InitAll(&ctx, "/etc/Glennergy-Fastigheter.json") != 0) {
        LOG_ERROR("Failed to initialize InputCache");
        log_Cleanup();
        return -1;
    }
    LOG_INFO("InputCache ready - entering event loop...");

    while(!SignalHandler_Stop())
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(ctx.meteo_fd, &read_fds);
        FD_SET(ctx.spotpris_fd, &read_fds);
        FD_SET(ctx.socket_fd, &read_fds);

        int max_fd = ctx.meteo_fd;
        if (ctx.spotpris_fd > max_fd)
            max_fd = ctx.spotpris_fd;
        if (ctx.socket_fd > max_fd)
            max_fd = ctx.socket_fd;

        struct timeval timeout = {1, 0}; // 1 second timeout to check for shutdown signal
        int ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready < 0) {
            if (errno == EINTR) {
                LOG_INFO("Select interrupted by signal, checking shutdown...");
                continue;
            }
            LOG_ERROR("select() error: %s", strerror(errno));
            break;
        }

        if (ready == 0)
            continue;
    
        if (FD_ISSET(ctx.meteo_fd, &read_fds)) {
            inputcache_HandleMeteoData(&ctx, ctx.meteo_fd);

            if (ctx.updated_meteo && ctx.updated_spotpris) {
                inputcache_SendNotification(NOTIFY_DATA_READY, (uint16_t)ctx.cache->meteo_count);
                LOG_INFO("Meteo updated, notified Algorithm");
            } else {
                LOG_INFO("Meteo received, waiting for spotpris data");
            }
        }

        if (FD_ISSET(ctx.spotpris_fd, &read_fds)) {
            inputcache_HandleSpotprisData(&ctx, ctx.spotpris_fd);

            if (ctx.updated_meteo && ctx.updated_spotpris) {
                inputcache_SendNotification(NOTIFY_DATA_READY, (uint16_t)ctx.cache->meteo_count);
                LOG_INFO("Spotpris updated, notified Algorithm");
            } else {
                LOG_INFO("Spotpris received, waiting for initial meteo data");
            }
        }

        if (FD_ISSET(ctx.socket_fd, &read_fds)) {
            int client_fd = accept(ctx.socket_fd, NULL, NULL);
            if (client_fd < 0) {
                LOG_ERROR("accept() failed: %s", strerror(errno));
            } else {
                inputcache_HandleRequest(&ctx, client_fd);
            }
        }
    }

    LOG_INFO("Shutting down Cache module...");
    
    inputcache_CleanupAll(&ctx);
    log_Cleanup();
    return ret;
}