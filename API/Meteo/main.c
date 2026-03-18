#define MODULE_NAME "MAIN"
#include "../../Server/Log/Logger.h"
#include "Meteo.h"
#include "../../Libs/Pipes.h"
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <fcntl.h>
#include <unistd.h>
#include "../../Libs/Utils/utils.h"

#define FIFO_METEO_WRITE "/tmp/fifo_meteo"

int main()
{

    curl_global_init(CURL_GLOBAL_DEFAULT);
    log_Init("meteo.log");
    LOG_INFO("Starting Meteo API...\n");

    if (mkfifo(FIFO_METEO_WRITE, 0666) < 0 && errno != EEXIST) {
        LOG_ERROR("Failed to create FIFO: %s", FIFO_METEO_WRITE);
        return -1;
    }
    LOG_INFO("FIFO ready: %s\n", FIFO_METEO_WRITE);

    setvbuf(stdout, NULL, _IONBF, 0);
    int meteo_fd_write = open(FIFO_METEO_WRITE, O_WRONLY);

    if (meteo_fd_write < 0)
    {
        LOG_ERROR("Failed to open file: %s\n", FIFO_METEO_WRITE);
        return -3;
    }

    LOG_INFO("nu börjar loopen\n");

    static time_t last_modified = -1;

    MeteoData data;
    Meteo_LoadGlennergy(&data);

    if (file_lastModified("/etc/Glennergy-Fastigheter.json", &last_modified) == 1)
    {
        Meteo_LoadGlennergy(&data);
        LOG_INFO("Info changed, reloaded file.\n");
    }

    meteo_Fetch(&data);
    LOG_INFO("Fetched meteo data, sending to cache...\n");
    ssize_t bytesWritten = Pipes_WriteBinary(meteo_fd_write, &data, sizeof(data));

    LOG_INFO("bytes skickade: %zd\n", bytesWritten);

    close(meteo_fd_write);
    log_Cleanup();
    return 0;
}