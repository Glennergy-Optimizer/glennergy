#include "Meteo.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>

constexpr const char* FIFO_METEO_WRITE = "/tmp/fifo_meteo";

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
    std::string configPath = "/etc/Glennergy-Fastigheter.json";

    // Fallback for local testing if /etc doesn't exist
    if (!std::filesystem::exists(configPath)) {
        configPath = "Glennergy-Fastigheter.json";
    }

    m.load(configPath);

    /*static std::filesystem::file_time_type last_modified;
    if (std::filesystem::exists(configPath)) {
        auto current_modified = std::filesystem::last_write_time(configPath);
        if (current_modified > last_modified) {
            m.load(configPath);
            std::cout << "Info changed, reloaded file.\n";
            last_modified = current_modified;
        }
    }*/

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
