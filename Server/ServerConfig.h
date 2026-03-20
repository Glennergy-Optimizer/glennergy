/**
 * @file ServerConfig.h
 * @brief Configuration structure and initialization for the server.
 * @defgroup ServerConfig Server Configuration
 * @{
 */

#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include "Log/Logger.h"

/**
 * @struct ServerConfig
 * @brief Holds server configuration parameters.
 * @note None of the members are dynamically allocated; ownership remains with caller.
 * @note config_path points to a string managed elsewhere; do not free here.
 */
typedef struct {
    int port;               /**< Server listening port */
    LogLevel log_level;     /**< Logging verbosity level */
    const char* config_path;/**< Path to configuration file, ownership external */
} ServerConfig;

/**
 * @brief Initializes the ServerConfig structure from command-line arguments.
 * @param[out] config Pointer to ServerConfig to initialize
 * @param[in] _Argv Command-line arguments array
 * @param[in] _Argc Number of command-line arguments
 * @pre config must be a valid, allocated pointer
 * @post ServerConfig members are set with defaults or argv values
 * @note If fewer than 2 arguments are provided, defaults (port=8080, log_level=LOG_LEVEL_INFO) are used
 */
void ServerConfig_Init(ServerConfig* config, char** _Argv, int _Argc);

#endif // SERVER_CONFIG_H

/** @} */