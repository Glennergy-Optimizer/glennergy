#define MODULE_NAME "ConnHandler"
#include <stdlib.h>
#include "ConnectionHandler.h"
#include "SignalHandler.h"
#include "../Log/Logger.h"

int ConnectionHandler_OnAccept(void *_Context, int _Socket);

void ConnectionHandler_Work(void *_Context, uint64_t monTime);

int ConnectionHandler_Initialize(ConnectionHandler **_ConnectionHandler, int _Port, Callback _Callback)
{
    ConnectionHandler *cHandler = (ConnectionHandler *)malloc(sizeof(ConnectionHandler));

    if (cHandler == NULL)
        return -1;

    TCPServer_Initialize(&cHandler->tcp_server, _Port, 100, ConnectionHandler_OnAccept, cHandler);

    TCPServer_Listen(cHandler->tcp_server);

    cHandler->client_add = _Callback;


    *_ConnectionHandler = cHandler;

    return 0;
}

int ConnectionHandler_OnAccept(void *_Context, int _Socket)
{
    ConnectionHandler *cHandler = (ConnectionHandler *)_Context;

    if (cHandler == NULL)
        return -1;

    LOG_INFO("New connection accepted");
    
    Connection* connection = NULL;
    if (Connection_Initialize(&connection, _Socket) != 0)
    {
        LOG_ERROR("Failed to initialize connection");
        return -2;
    }

    if (cHandler->client_add(connection) < 0)
    {
        LOG_ERROR("Failed to add connection to queue");
        Connection_Dispose(&connection);
        return -3;
    }

    return 0;
}

void ConnectionHandler_Dispose(ConnectionHandler **_ConnectionHandler)
{
    if (_ConnectionHandler == NULL || *_ConnectionHandler == NULL)
        return;

    ConnectionHandler *cHandler = *_ConnectionHandler;


    TCPServer_Dispose(&cHandler->tcp_server);
    free(cHandler);
    cHandler = NULL;
}