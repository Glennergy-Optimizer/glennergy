/**
 * @file spotpristest.c
 * @brief Test entry point for the Spotpris module.
 *
 * @ingroup SPOTPRIS
 *
 * Fetches spot price data, then forwards the resulting structure through the
 * FIFO used by the InputCache process.
 *
 * @note Intended for testing and integration rather than core business logic.
 * @warning Writing to the FIFO blocks until a reader connects.
 */

#define MODULE_NAME "MAIN"

#include "../../Server/Log/Logger.h"
#include "Spotpris.h"
#include <stdio.h>
#include "../../Libs/Pipes.h"
#include "../../Libs/GlennergyPaths.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Named pipe used for IPC between Spotpris module and InputCache.
 */
#define FIFO_SPOTPRIS_WRITE GLENNERGY_SPOTPRIS_FIFO_PATH

/**
 * @brief Program entry point.
 *
 * Initializes logging, fetches spot price data, and writes the binary result
 * to the configured FIFO.
 *
 * @return
 * - 0 on success
 * - Negative error code on failure
 *
 * @warning This function performs blocking I/O when writing to the FIFO.
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
    LOG_INFO("Using FIFO: %s\n", FIFO_SPOTPRIS_WRITE);

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
