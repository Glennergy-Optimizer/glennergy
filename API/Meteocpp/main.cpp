/**
 * @file main.cpp
 * @brief Entry point for Meteo C++ service.
 *
 * @ingroup MeteoCppModule
 */

#include "Meteo.hpp"
#include "../../Libs/GlennergyPaths.h"
#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>

constexpr const char* FIFO_METEO_WRITE = GLENNERGY_METEO_FIFO_PATH;

/**
 * @brief Main entry point.
 *
 * @return 0 on success, negative on failure
 *
 * @pre FIFO must exist or be created externally
 * @post Meteo data written to FIFO
 *
 * @warning Blocking I/O operations
 */
int main()
{
    std::cout << "Starting Meteo API...\n";

    std::cout << "Waiting for FIFO: " << FIFO_METEO_WRITE << "\n";
    int meteo_fd_write = open(FIFO_METEO_WRITE, O_WRONLY);

    if (meteo_fd_write < 0)
    {
        std::cerr << "Failed to open FIFO: " << FIFO_METEO_WRITE << "\n";
        return -3;
    }

    std::cout << "Start\n";

    meteocpp::meteo m;
    std::string configPath = GLENNERGY_CONFIG_PATH;

    if (!m.load(configPath)) {
        std::cerr << "Failed to load configuration: " << configPath << "\n";
        close(meteo_fd_write);
        return -2;
    }

    if (!m.fetchAll()) {
        std::cerr << "Failed to fetch meteo data\n";
        close(meteo_fd_write);
        return -1;
    }

    std::cout << "Fetched meteo data, sending to cache...\n";

    auto& data = m.data();
    ssize_t bytesWritten = 0;
    size_t total = 0;
    size_t dataSize = sizeof(data);
    const char* buffer = reinterpret_cast<const char*>(&data);

    while (total < dataSize) {
        bytesWritten = write(meteo_fd_write, buffer + total, dataSize - total);

        if (bytesWritten > 0) {
            total += bytesWritten;
        }

        if (bytesWritten < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) break; // Or handle blocked pipe
            std::cerr << "Failed to write to FIFO (errno: " << errno << ")\n";
            break;
        }
    }

    std::cout << "Bytes sent: " << total << " (expected: " << dataSize << ")\n";

    close(meteo_fd_write);
    return 0;
}
