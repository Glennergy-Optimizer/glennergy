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



// Enkelkommando för att kompilera just denna filen Todo - Can be removed?
// gcc -Wall -Wextra -std=c11 spotpristest.c spotpris.c ../Fetcher.c -lcurl -ljansson -D_POSIX_C_SOURCE=200112L -o spotpris_app

// Named pipe
#define FIFO_SPOTPRIS_WRITE "/tmp/fifo_spotpris"

int main(void)
{
    log_Init("spotpris.log");
    curl_global_init(CURL_GLOBAL_DEFAULT);

    AllaSpotpriser spotpriser;
    LOG_INFO("Fetching spotpris data...\n");
    int rc = Spotpris_FetchAll(&spotpriser);
    if (rc != 0)
    {
        LOG_ERROR("Failed to fetch: %d\n", rc);
        return -4;
    }

    // Todo - Endast för debugging/testing, ta bort i prod
   // AllaSpotpriser_Print(&spotpriser);


    if (mkfifo(FIFO_SPOTPRIS_WRITE, 0666) < 0 && errno != EEXIST)
    {
        LOG_ERROR("Failed to create FIFO: %s", FIFO_SPOTPRIS_WRITE);
        return -1;
    }
    
    LOG_INFO("FIFO ready: %s\n", FIFO_SPOTPRIS_WRITE);
    int spotpris_fd_write = open(FIFO_SPOTPRIS_WRITE, O_WRONLY);

    if (spotpris_fd_write < 0)
    {
        LOG_ERROR("Failed to open file: %s\n", FIFO_SPOTPRIS_WRITE);
        return -3;
    }
    LOG_INFO("Sending spotpris data to cache...\n");    
    ssize_t bytesWritten = Pipes_WriteBinary(spotpris_fd_write, &spotpriser, sizeof(spotpriser));
    
    LOG_INFO("bytes skickade: %zd\n", bytesWritten);
        
    curl_global_cleanup();
    close(spotpris_fd_write);
    log_Cleanup();
    return 0;
}