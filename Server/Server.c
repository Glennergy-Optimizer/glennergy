#define MODULE_NAME "SERVER"
#define _XOPEN_SOURCE 500
/**
 * @file Server.c
 * @brief Implementation of the Server module.
 *
 * @ingroup Server
 */

#include "Server.h"
#include "SignalHandler.h"
#include "../Libs/Utils/utils.h"
#include "../Libs/Threads.h"
#include "Log/Logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Implementation of Server_Initialize.
 *
 * See header for full contract documentation.
 */
int Server_Initialize(Server **_Server, char **_Argv, int _Argc)
{
    Server *srv = (Server *)malloc(sizeof(Server));
    if (srv == NULL)
        return -1;

    ServerConfig_Init(&srv->config, _Argv, _Argc);

    printf("Port: %d\n", srv->config.port);
    printf("Log level %d\n", srv->config.log_level);

    *_Server = srv;
    return 0;
}

/**
 * @brief Add Crontab entry using a shell script.
 *
 * @note Forks a child process to execute "./crontab_inst.sh add".
 * @post Waits for child to complete.
 */
void Crontab_Add()
{
}

/**
 * @brief Remove Crontab entry using a shell script.
 *
 * @note Forks a child process to execute "./crontab_inst.sh remove".
 * @post Waits for child to complete.
 */
void Crontab_Remove()
{
}

/**
 * @brief Implementation of Server_Run.
 *
 * See header for full contract documentation.
 */
int Server_Run(Server *_Server)
{
    SignalHandler_Initialize();

    Threads threads[POOL_SIZE];
    if (Threads_Initialize(threads) != 0)
        return -1;

    if (smw_init() != 0)
    {
        Threads_Dispose(threads);
        return -1;
    }

    ConnectionHandler *cHandler = NULL;
    if (ConnectionHandler_Initialize(&cHandler, _Server->config.port, Threads_AddQueueItem) != 0)
    {
        smw_dispose();
        Threads_Dispose(threads);
        return -1;
    }

    while (SignalHandler_Stop() == 0)
    {
        uint64_t monTime = SystemMonotonicMS();
        smw_work(monTime);
        usleep(100000); // Todo från compiler warning - Byta till använda "nanosleep" från "time.h" istället för "usleep" från "unistd.h"?
    }

    ConnectionHandler_Dispose(&cHandler);
    smw_dispose();
    Threads_Dispose(threads);

    return 0;
}

/**
 * @brief Implementation of Server_Dispose.
 *
 * See header for full contract documentation.
 */
void Server_Dispose(Server **_Server)
{
    if (_Server == NULL || *_Server == NULL)
        return;

    Server *srv = *_Server;

    free(srv);
    srv = NULL;
}
