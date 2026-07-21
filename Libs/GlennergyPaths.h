#ifndef GLENNERGY_PATHS_H
#define GLENNERGY_PATHS_H

/**
 * @file GlennergyPaths.h
 * @brief Defines the shared file and socket paths used by Glennergy.
 *
 * @defgroup GLENNERGY_PATHS GLENNERGY_PATHS
 * @brief Shared path constants for configuration and runtime IPC.
 *
 * Centralizes the filesystem locations used by the application so all modules
 * refer to the same configuration file, runtime directory, FIFO paths, and
 * cache socket path.
 * @{
 */

#define GLENNERGY_CONFIG_PATH "/etc/glennergy/fastigheter.json"

#define GLENNERGY_RUNTIME_DIR "/run/glennergy"
#define GLENNERGY_METEO_FIFO_PATH GLENNERGY_RUNTIME_DIR "/meteo.fifo"
#define GLENNERGY_SPOTPRIS_FIFO_PATH GLENNERGY_RUNTIME_DIR "/spotpris.fifo"
#define GLENNERGY_CACHE_SOCKET_PATH GLENNERGY_RUNTIME_DIR "/cache.sock"

/** @} */

#endif
