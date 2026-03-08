#define MODULE_NAME "SERVER"
#include "Log/Logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "Server.h"
#include "SignalHandler.h"
#include "../Libs/Utils/utils.h"
#include "../Libs/Threads.h"
#include "Utils/APIHandler.h"

int Server_Initialize(Server **_Server, char **_Argv, int _Argc)
{
    Server *srv = (Server *)malloc(sizeof(Server));

    if (srv == NULL)
        return -1;

    ServerConfig_Init(&srv->config, _Argv, _Argc);
    printf("Port_ %d\n", srv->config.port);
    printf("Log level %d\n", srv->config.log_level);
    *_Server = srv;
    return 0;
}

int Server_Run(Server *_Server)
{

    SignalHandler_Initialize();

    int status_server, status_cache, status_algo;
    pid_t pid_server = fork();

    if (pid_server < 0)
    {
        exit(EXIT_FAILURE);
    }
    else if (pid_server == 0)
    {

        Threads threads[POOL_SIZE];
        Threads_Initialize(threads);

        smw_init();

        APIHandler_t *api_handler = NULL;
        APIHandler_Init(&api_handler);

        ConnectionHandler *cHandler = NULL;
        ConnectionHandler_Initialize(&cHandler, _Server->config.port, Threads_AddQueueItem, api_handler);

        uint64_t monTime = 0;
        while (SignalHandler_Stop() == 0)
        {
            monTime = SystemMonotonicMS();
            smw_work(monTime);
            usleep(100000); // Todo från compiler warning - Byta till använda "nanosleep" från "time.h" istället för "usleep" från "unistd.h"?
        }

        ConnectionHandler_Dispose(&cHandler);
        smw_dispose();
        APIHandler_Dispose(&api_handler);
        Threads_Dispose(threads);

        log_CloseWrite();
        exit(EXIT_SUCCESS);
    }

    pid_t pid_cache = fork();

    if (pid_cache < 0)
    {
        kill(pid_server, SIGTERM);
        exit(EXIT_FAILURE);
    }
    else if (pid_cache == 0)
    {
        execlp("Glennergy-InputCache", "Glennergy-InputCache", NULL);
        LOG_ERROR("Failed to execute Glennergy-InputCache");
        exit(EXIT_FAILURE);
    }
    // else
    // {
    //     test_reader();
    //     wait(&status_pid);
    //     wait(&status_cache);
    // }
    pid_t pid_algo = fork();
    if (pid_algo < 0)
    {
        LOG_ERROR("Failed to fork for Glennergy-Algorithm");
        kill(pid_server, SIGTERM);
        kill(pid_cache, SIGTERM);       //handle restart instead of kill?
        exit(EXIT_FAILURE);
    }
    else if (pid_algo == 0)
    {
        execlp("Glennergy-Algorithm", "Glennergy-Algorithm", NULL);
        LOG_ERROR("Failed to execute Glennergy-Algorithm");
        exit(EXIT_FAILURE);
    }

    LOG_INFO("All processes started: Server PID: %d, Cache PID: %d, Algorithm PID: %d", pid_server, pid_cache, pid_algo);
    waitpid(pid_server, &status_server, 0);
    waitpid(pid_cache, &status_cache, 0);
    waitpid(pid_algo, &status_algo, 0);

    return 0;
}

void Server_Dispose(Server **_Server)
{
    if (_Server == NULL || *_Server == NULL)
        return;

    Server *srv = *_Server;

    free(srv);
    srv = NULL;
}