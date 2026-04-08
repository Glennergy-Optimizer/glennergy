/**
 * @file spotpristest.c
 * @brief Entry point for the Spotpris module test harness.
 *
 * @ingroup SPOTPRIS
 *
 * Fetches spot prices, then sends the resulting data through a FIFO to the
 * consumer process used by the integration test path.
 *
 * @note Intended for testing and integration.
 * @warning The FIFO write blocks until a reader connects.
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
 * @brief Named pipe used for IPC between Spotpris module and InputCache.
 */
#define FIFO_SPOTPRIS_WRITE "/tmp/fifo_spotpris"

/**
 * @brief Program entry point.
 *
 * Initializes logging, fetches spot price data, creates the FIFO if needed,
 * and writes the full structure to the pipe.
 *
 * @return
 * - 0 on success
 * - Negative error code on failure
 *
 * @note Performs network I/O and blocking FIFO I/O.
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
     * @param spotpris_fd_write File descriptor of FIFO.
     * @param &spotpriser Pointer to data structure.
     * @param sizeof(spotpriser) Size of data structure.
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
