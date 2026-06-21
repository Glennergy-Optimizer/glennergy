/**
 * @file spotpristest.c
 * @brief Entry point / test for Spotpris module.
 *
 * @ingroup SPOTPRIS
 *
 * Fetches spot price data using the Spotpris module and sends it via a named
 * pipe to the InputCache consumer for testing and integration purposes.
 *
 * @note Performs network requests, blocking FIFO I/O, and writes logs to file.
 * @warning Blocks on FIFO open and write if no reader is connected.
 */

#define MODULE_NAME "MAIN"

#include "../../Server/Log/Logger.h"
#include "Spotpris.h"
#include <stdio.h>
#include "../../Libs/Pipes.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

/**
 * @brief Named pipe path used for IPC between Spotpris module and InputCache.
 */
#define FIFO_SPOTPRIS_WRITE "/tmp/fifo_spotpris"

/**
 * @brief Runs the Spotpris test flow.
 *
 * Initializes logging and libcurl, fetches spot prices for all SE areas,
 * ensures the FIFO exists, opens it for writing, and sends the complete
 * AllaSpotpriser structure over the pipe before cleaning up resources.
 *
 * @return
 * - 0 on success
 * - -4 if fetching spot prices fails
 * - -3 if FIFO open fails
 * - -1 if FIFO creation fails
 *
 * @note Intended for testing and integration; not part of core business logic.
 * @warning Performs blocking I/O on FIFO operations and network I/O via libcurl.
 */
int main(void)
{
    log_Init("spotpris.log");

    // Global initialization for libcurl (required before any curl usage)
    curl_global_init(CURL_GLOBAL_DEFAULT);

    AllaSpotpriser spotpriser;

    LOG_INFO("Fetching spotpris data...\n");

    int rc = Spotpris_FetchAll(&spotpriser);
    if (rc != 0)
    {
        LOG_ERROR("Failed to fetch: %d\n", rc);
        return -4;
    }

    // Debug helper (disabled in production)
    // AllaSpotpriser_Print(&spotpriser);

    /**
     * Create FIFO if it does not exist.
     *
     * @note mkfifo will fail with EEXIST if already created (expected case).
     * @warning Fails if permissions prevent creation.
     */
    if (mkfifo(FIFO_SPOTPRIS_WRITE, 0666) < 0 && errno != EEXIST)
    {
        LOG_ERROR("Failed to create FIFO: %s", FIFO_SPOTPRIS_WRITE);
        return -1;
    }

    LOG_INFO("FIFO ready: %s\n", FIFO_SPOTPRIS_WRITE);

    /**
     * Open FIFO for writing.
     *
     * @warning This call will BLOCK until a reader connects.
     */
    int spotpris_fd_write = open(FIFO_SPOTPRIS_WRITE, O_WRONLY);

    if (spotpris_fd_write < 0)
    {
        LOG_ERROR("Failed to open file: %s\n", FIFO_SPOTPRIS_WRITE);
        return -3;
    }

    LOG_INFO("Sending spotpris data to cache...\n");

    /**
     * Write binary struct to pipe.
     *
     * @note Writes entire AllaSpotpriser struct in one operation.
     * @warning Receiver must use identical struct layout (ABI-sensitive).
     */
    ssize_t bytesWritten =
        Pipes_WriteBinary(spotpris_fd_write, &spotpriser, sizeof(spotpriser));

    LOG_INFO("bytes skickade: %zd\n", bytesWritten);

    // Cleanup global curl state
    curl_global_cleanup();

    // Close FIFO descriptor
    close(spotpris_fd_write);

    // Shutdown logging system
    log_Cleanup();

    return 0;
}
