#define MODULE_NAME "MAIN"
#include "../Server/Log/Logger.h"
#include "InputCache.h"
#include "../Libs/Pipes.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../Server/SignalHandler.h"

int main()
{
    log_Init("cache.log");
    SignalHandler_Initialize();
    LOG_INFO("Starting Cache module...");

    InputCache_t *cache = malloc(sizeof(InputCache_t));
    if (!cache) {
        LOG_ERROR("malloc() Failed to allocate memory for InputCache");
        return -1;
    }
    memset(cache, 0, sizeof(InputCache_t));

    if (inputcache_Init(cache, "/etc/Glennergy-Fastigheter.json") != 0) {
        LOG_ERROR("Failed to initialize InputCache");
        free(cache);
        return -1;
    }

    int meteo_fd, spotpris_fd;
    if (inputcache_OpenFIFOs(&meteo_fd, &spotpris_fd) != 0) {
        LOG_ERROR("Failed to open FIFOs");
        free(cache);
        return -1;
    }

    int socket_fd = inputcache_CreateSocket();
    if (socket_fd < 0) {
        LOG_ERROR("Failed to create socket");
        close(meteo_fd);
        close(spotpris_fd);
        free(cache);
        return -1;
    }

    LOG_INFO("InputCache ready - entering event loop...");

    while(!SignalHandler_Stop())
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(meteo_fd, &read_fds);
        FD_SET(spotpris_fd, &read_fds);
        FD_SET(socket_fd, &read_fds);

        int max_fd = meteo_fd;
        if (spotpris_fd > max_fd) max_fd = spotpris_fd;
        if (socket_fd > max_fd) max_fd = socket_fd;

        int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ready < 0) {
            LOG_ERROR("select() error: %s", strerror(errno));
            break;
        }
        if (FD_ISSET(meteo_fd, &read_fds)) {
            inputcache_HandleMeteoData(cache, meteo_fd);
        }
        if (FD_ISSET(spotpris_fd, &read_fds)) {
            inputcache_HandleSpotprisData(cache, spotpris_fd);
        }
        if (FD_ISSET(socket_fd, &read_fds)) {
            int client_fd = accept(socket_fd, NULL, NULL);
            if (client_fd < 0) {
                LOG_ERROR("accept() failed: %s", strerror(errno));
            } else {
                inputcache_HandleRequest(cache, client_fd);
            }
        }
    }


    LOG_INFO("Shutting down Cache module...");
    close(meteo_fd);
    close(spotpris_fd);
    close(socket_fd);
    
    printf("Cleaned up input cache\n");
    free(cache);
    log_Cleanup();
    return 0;
}