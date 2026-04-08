/**
 * @file spotpristest.c
 * @brief Entry point for the Spotpris module test harness.
 *
 * This program:
 * 1. Initializes logging and global dependencies (libcurl)
 * 2. Fetches spot price data using Spotpris module
 * 3. Sends the data via FIFO to another process (InputCache)
 *
 * @ingroup SPOTPRIS
 *
 * --- Runtime Behavior
 * - Fetches spot prices for all SE areas (SE1–SE4)
 * - Combines today + tomorrow data
 * - Serializes and sends full struct via binary pipe
 *
 * --- Side Effects
 * - Creates FIFO at `/tmp/fifo_spotpris`
 * - Performs blocking I/O (FIFO write)
 * - Performs network requests (via Spotpris module)
 * - Writes logs to `spotpris.log`
 *
 * --- Dependencies
 * - libcurl (requires curl_global_init / cleanup)
 * - FIFO consumer must exist (reader process)
 * - Spotpris module must be functional
 *
 * --- Error Handling
 * - Returns -4 if fetch fails
 * - Returns -3 if FIFO open fails
 * - Returns -1 if FIFO creation fails
 *
 * @note This file is intended for testing and integration.
 * @note Not part of core business logic.
 * @warning Blocking behavior occurs if no FIFO reader is connected.
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
 * @file spotpristest.c
 * @brief Test entry point for the Spotpris module.
 *
 * @ingroup SPOTPRIS
 *
 * Initializes logging, fetches spot prices, and writes the resulting
 * structure to the configured FIFO for integration with the consumer.
 *
 * @note Performs network I/O and blocking FIFO I/O.
 * @warning The FIFO open call blocks until a reader connects.
 */

/**
 * @brief Program entry point.
 *
 * Coordinates initialization, data fetching, and IPC pipeline.
 *
 * Execution flow:
 * - Initialize logging
 * - Initialize libcurl
 * - Fetch spot price data
 * - Create/open FIFO
 * - Send data via pipe
 * - Cleanup resources
 *
 * @return
 * - 0 on success
 * - Negative error code on failure
 *
 * @pre Logging system must be functional.
 * @pre Spotpris module must be functional.
 * @post Spot price data is sent via FIFO to consumer.
 * @warning This function performs blocking I/O when writing to FIFO.
 * @note Intended for testing; does not affect core business logic.
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
