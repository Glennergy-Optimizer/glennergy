/**
 * @file spotpristest.c
 * @brief Test entry point for Spotpris data fetch and FIFO transfer.
 *
 * @ingroup SPOTPRIS
 *
 * Fetches spot price data for all SE areas, combines today and tomorrow,
 * and sends the full struct to a FIFO consumer.
 *
 * @note Intended for testing and integration, not core business logic.
 * @warning FIFO open/write can block until a reader connects.
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
 * Initializes logging and libcurl, fetches spot prices, and writes the
 * serialized struct to the FIFO.
 *
 * @return
 * - 0 on success
 * - Negative error code on failure
 *
 * @note Intended for testing; does not affect core business logic.
 * @warning FIFO open/write can block until a reader connects.
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

    // Create FIFO if it does not exist; EEXIST is an expected case.
    if (mkfifo(FIFO_SPOTPRIS_WRITE, 0666) < 0 && errno != EEXIST)
    {
        LOG_ERROR("Failed to create FIFO: %s", FIFO_SPOTPRIS_WRITE);
        return -1;
    }

    LOG_INFO("FIFO ready: %s\n", FIFO_SPOTPRIS_WRITE);

    // Open FIFO for writing; this call blocks until a reader connects.
    int spotpris_fd_write = open(FIFO_SPOTPRIS_WRITE, O_WRONLY);

    if (spotpris_fd_write < 0)
    {
        LOG_ERROR("Failed to open file: %s\n", FIFO_SPOTPRIS_WRITE);
        return -3;
    }

    LOG_INFO("Sending spotpris data to cache...\n");

    // Write the full AllaSpotpriser struct; receiver must match ABI layout.
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
