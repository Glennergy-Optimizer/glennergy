#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Server.h"
#include "../Algorithm/testreader.h"

int main(int argc, char* argv[]) {
    log_Init("server.log");

    printf("Server is starting...\n");
    Server* server = NULL;
    Server_Initialize(&server, argv, argc);
    
    log_SetLevel(server->config.log_level);
    printf("Log level set to: %s\n", log_GetLevelString(server->config.log_level));
    
    Server_Run(server);
    
    Server_Dispose(&server);
    
    printf("Server is shutting down\n");
    log_Cleanup();
    return 0;
}