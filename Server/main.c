/**
 * @file main.c
 * @brief Entry point for the server application.
 * @ingroup Server
 *
 * Initializes logging, starts the server, runs the main loop, 
 * and disposes of server resources upon termination.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Server.h"

int main(int argc, char* argv[]) {
    log_Init(NULL);

    printf("Server is starting...\n");
    Server* server = NULL;

    // Initialize server from command-line arguments
    if (Server_Initialize(&server, argv, argc) != 0) {
        fprintf(stderr, "Failed to initialize server\n");
        log_Cleanup();
        return EXIT_FAILURE;
    }
    
    // Set logging level
    log_SetLevel(server->config.log_level);
    printf("Log level set to: %s\n", log_GetLevelString(server->config.log_level));
    
    // Run server main loop (blocking)
    int result = Server_Run(server);
    
    // Dispose server and free resources
    Server_Dispose(&server);
    
    printf("Server is shutting down\n");
    log_Cleanup();
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

// Suggestion: Could add command-line argument parsing for more configuration options
// Suggestion: Could handle signals to gracefully terminate server before main exits
