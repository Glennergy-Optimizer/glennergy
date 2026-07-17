/**
 * @file main.c
 * @brief Entry point for InputCache service.
 *
 * @details
 * Event-driven service that:
 * - Listens to Meteo and Spotpris FIFOs
 * - Accepts client connections via UNIX socket
 * - Dispatches requests and updates cache state
 *
 * Uses select() for multiplexing IPC channels.
 *
 * @note This is the main executable for the InputCache module.
 * @warning Blocking I/O may occur in select() and accept().
 * @warning Exits on SIGINT/SIGTERM via SignalHandler.
 * @pre Input FIFOs must exist and be writable by this process.
 * @pre Configuration file must exist at specified path.
 * @post Allocated InputCache structure is freed on exit.
 * @post All opened file descriptors are closed on exit.
 */

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

/**
 * @brief Program entry point for InputCache.
 *
 * Execution flow:
 * - Initializes logging
 * - Sets up signal handling
 * - Allocates and initializes cache
 * - Opens FIFOs for Meteo and Spotpris data
 * - Creates a UNIX socket for client requests
 * - Enters an event loop multiplexing IPC and client sockets
 * - Handles incoming data and client requests
 * - Cleans up resources on termination
 *
 * @return
 * - 0 on normal exit
 * - Negative value on failure
 *
 * @pre Configuration file path must be valid.
 * @pre FIFOs must exist for reading.
 * @post All resources (memory, FIFOs, sockets) are released.
 * @warning Event loop may block on select().
 */
int main()
{
    log_Init("cache.log");
    SignalHandler_Initialize();
    LOG_INFO("Starting Cache module...");

    // Allocate and zero InputCache structure
    InputCache_t *cache = malloc(sizeof(InputCache_t));
    if (!cache) {
        LOG_ERROR("malloc() Failed to allocate memory for InputCache");
        return -1;
    }
    memset(cache, 0, sizeof(InputCache_t));

    // Initialize InputCache with configuration
    if (inputcache_Init(cache, GLENNERGY_CONFIG_PATH) != 0) {
        LOG_ERROR("Failed to initialize InputCache");
        free(cache);
        return -1;
    }

    // Open FIFOs for incoming data
    int meteo_fd, spotpris_fd;
    if (inputcache_OpenFIFOs(&meteo_fd, &spotpris_fd) != 0) {
        LOG_ERROR("Failed to open FIFOs");
        free(cache);
        return -1;
    }

    // Create UNIX socket for client requests
    int socket_fd = inputcache_CreateSocket();
    if (socket_fd < 0) {
        LOG_ERROR("Failed to create socket");
        close(meteo_fd);
        close(spotpris_fd);
        free(cache);
        return -1;
    }

    LOG_INFO("InputCache ready - entering event loop...");

    // Event loop: multiplex FIFOs and client socket
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

        // Handle incoming Meteo data
        if (FD_ISSET(meteo_fd, &read_fds)) {
            inputcache_HandleMeteoData(cache, meteo_fd);
        }

        // Handle incoming Spotpris data
        if (FD_ISSET(spotpris_fd, &read_fds)) {
            inputcache_HandleSpotprisData(cache, spotpris_fd);
        }

        // Handle incoming client requests
        if (FD_ISSET(socket_fd, &read_fds)) {
            int client_fd = accept(socket_fd, NULL, NULL);
            if (client_fd < 0) {
                LOG_ERROR("accept() failed: %s", strerror(errno));
            } else {
                inputcache_HandleRequest(cache, client_fd);
            }
        }
    }

    // Cleanup
    LOG_INFO("Shutting down Cache module...");
    close(meteo_fd);
    close(spotpris_fd);
    close(socket_fd);

    printf("Cleaned up input cache\n");
    free(cache);
    log_Cleanup();

    return 0;
}
