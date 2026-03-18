#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include "Log/Logger.h"

typedef struct {
    int port;
    LogLevel log_level;
    const char* config_path;
} ServerConfig;

void ServerConfig_Init(ServerConfig* config, char** _Argv, int _Argc);

#endif