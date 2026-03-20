/**
 * @file ServerConfig.c
 * @brief Implementation of server configuration initialization.
 * @ingroup ServerConfig
 */

#include "ServerConfig.h"
#include <stddef.h>
#include <stdlib.h>

/**
 * @brief Initializes the ServerConfig structure from command-line arguments.
 * @param[out] config Pointer to ServerConfig to initialize
 * @param[in] _Argv Command-line arguments array
 * @param[in] _Argc Number of command-line arguments
 * @pre config must be a valid, allocated pointer
 * @post ServerConfig members are set according to argv or defaults
 * @note Defaults: port=8080, log_level=LOG_LEVEL_INFO
 * @note Unused arguments are ignored
 */
void ServerConfig_Init(ServerConfig* config, char** _Argv, int _Argc)
{
    if(_Argc > 1)
    {
        config->port = strtol(_Argv[1], NULL, 10);
        if(_Argc > 2)
        {
            config->log_level = strtol(_Argv[2], NULL, 10);
        }
        else
        {
            config->log_level = LOG_LEVEL_INFO;
        }
    }
    else
    {
        config->log_level = LOG_LEVEL_INFO;
        config->port = 8080;
    }
}