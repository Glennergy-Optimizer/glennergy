#define MODULE_NAME "ConnHandler"
/**
 * @file ConnectionHandler.c
 * @brief Implementation of the ConnectionHandler module.
 *
 * @ingroup ConnectionHandler
 */

#include <stdlib.h>
#include "ConnectionHandler.h"
#include "SignalHandler.h"
#include "../Log/Logger.h"

/**
 * @brief Handles accepted TCP connections.
 *
 * Creates a Connection instance and forwards it to the registered callback.
 *
 * @param _Context Pointer to the ConnectionHandler instance.
 * @param _Socket Accepted socket descriptor.
 *
 * @return
 * - 0 on success
 * - -1 if the context is invalid
 * - -2 if the connection cannot be initialized
 * - -3 if the connection cannot be queued
 */
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

    // Suggestion: Could initialize connection->bytesReadOut to 0 here to avoid uninitialized value

    if (cHandler->client_add(connection) < 0)
    {
        LOG_ERROR("Failed to add connection to queue");
        Connection_Dispose(&connection);
        return -3;
    }

    return 0;
}

/**
 * @brief Periodic work callback for ConnectionHandler.
 *
 * Currently unused.
 *
 * @param _Context Unused context pointer.
 * @param monTime Monotonic time value.
 */
void ConnectionHandler_Work(void *_Context, uint64_t monTime)
{
    // Suggestion: Could periodically cleanup stale connections if needed
}

/**
 * @brief Implementation of ConnectionHandler_Initialize.
 *
 * See header for full contract documentation.
 */
int ConnectionHandler_Initialize(ConnectionHandler **_ConnectionHandler, int _Port, Callback _Callback)
{
    ConnectionHandler *cHandler = (ConnectionHandler *)malloc(sizeof(ConnectionHandler));
    if (cHandler == NULL)
        return -1;

    cHandler->client_add = _Callback;
    // Suggestion: Could validate _Callback is not NULL before assignment

    if (TCPServer_Initialize(&cHandler->tcp_server, _Port, 100, ConnectionHandler_OnAccept, cHandler) != 0)
    {
        free(cHandler);
        return -1;
    }

    if (TCPServer_Listen(cHandler->tcp_server) != 0)
    {
        TCPServer_Dispose(&cHandler->tcp_server);
        free(cHandler);
        return -1;
    }

    *_ConnectionHandler = cHandler;
    return 0;
}

/**
 * @brief Implementation of ConnectionHandler_Dispose.
 *
 * See header for full contract documentation.
 */
void ConnectionHandler_Dispose(ConnectionHandler **_ConnectionHandler)
{
    if (_ConnectionHandler == NULL || *_ConnectionHandler == NULL)
        return;

    ConnectionHandler *cHandler = *_ConnectionHandler;

    TCPServer_Dispose(&cHandler->tcp_server);
    free(cHandler);
    cHandler = NULL;
}
