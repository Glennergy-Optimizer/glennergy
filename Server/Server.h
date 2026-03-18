#ifndef SERVER_H
#define SERVER_H


#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Connection/ConnectionHandler.h"
#include "ServerConfig.h"

typedef struct
{
    ConnectionHandler cHandler;
    ServerConfig config;
    int port;

} Server;

int Server_Initialize(Server** _Server, char** _Argv, int _Argc);


int Server_Run(Server* _Server);


void Server_Dispose(Server** _Server);


#endif