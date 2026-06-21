/**
 * @file spotpristest.c
 * @brief Test entry point for the Spotpris module IPC flow.
 *
 * @ingroup SPOTPRIS
 *
 * Initializes logging and libcurl, fetches spot price data via the Spotpris
 * module, then writes the resulting binary struct to a named FIFO for
 * consumption by another process (e.g., InputCache).
 *
 * @note Intended for testing/integration; not part of core business logic.
 * @warning Opening and writing the FIFO may block until a reader is connected.
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

/** Named pipe used for IPC between Spotpris module and its consumer. */
#define FIFO_SPOTPRIS_WRITE "/tmp/fifo_spotpris"

/**
 * @brief Runs the Spotpris test harness.
 *
 * Fetches all spot prices into an AllaSpotpriser struct and writes the raw
 * binary representation to @ref FIFO_SPOTPRIS_WRITE.
 *
 * @return 0 on success, otherwise a negative error code.
 *
 * @note See the Spotpris module for fetch semantics and data population rules.
 * @warning The FIFO transfer is ABI-sensitive; reader and writer must agree on
 *          the struct layout.
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

    // Create FIFO if it does not exist.
    // mkfifo will fail with EEXIST if already created (expected case).
    if (mkfifo(FIFO_SPOTPRIS_WRITE, 0666) < 0 && errno != EEXIST)
    {
        LOG_ERROR("Failed to create FIFO: %s", FIFO_SPOTPRIS_WRITE);
        return -1;
    }

    LOG_INFO("FIFO ready: %s\n", FIFO_SPOTPRIS_WRITE);

    // Open FIFO for writing.
    // This call will BLOCK until a reader connects.
    int spotpris_fd_write = open(FIFO_SPOTPRIS_WRITE, O_WRONLY);

    if (spotpris_fd_write < 0)
    {
        LOG_ERROR("Failed to open file: %s\n", FIFO_SPOTPRIS_WRITE);
        return -3;
    }

    LOG_INFO("Sending spotpris data to cache...\n");

    // Write binary struct to pipe.
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
