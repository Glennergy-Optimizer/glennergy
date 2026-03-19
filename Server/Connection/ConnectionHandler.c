#define MODULE_NAME "ConnHandler"
/**
 * @file ConnectionHandler.c
 * @brief Implementation of ConnectionHandler for managing multiple TCP connections.
 *
 * Handles accepting connections, invoking callbacks, and disposing resources.
 * Follows the customized Doxygen standard with `// Suggestion:` comments for improvements.
 *
 * @author YourName
 * @date 2026-03-19
 */

#include <stdlib.h>
#include "ConnectionHandler.h"
#include "SignalHandler.h"
#include "../Log/Logger.h"

/**
 * @brief Internal handler for accepted connections.
 *
 * @param _Context Pointer to ConnectionHandler context.
 * @param _Socket Socket file descriptor for the new connection.
 * @return 0 on success, negative on error.
 */
int ConnectionHandler_OnAccept(void *_Context, int _Socket);

/**
 * @brief Periodic work function for ConnectionHandler.
 *
 * @param _Context Pointer to ConnectionHandler context.
 * @param monTime Current monotonic time in milliseconds.
 */
void ConnectionHandler_Work(void *_Context, uint64_t monTime);

/**
 * @brief Initialize a ConnectionHandler instance and start listening.
 *
 * @param _ConnectionHandler Pointer to pointer to allocate.
 * @param _Port TCP port for the server.
 * @param _Callback Callback function for new connections.
 * @return 0 on success, negative on error.
 */
int ConnectionHandler_Initialize(ConnectionHandler **_ConnectionHandler, int _Port, Callback _Callback)
{
    ConnectionHandler *cHandler = (ConnectionHandler *)malloc(sizeof(ConnectionHandler));
    if (cHandler == NULL)
        return -1;

    TCPServer_Initialize(&cHandler->tcp_server, _Port, 100, ConnectionHandler_OnAccept, cHandler);
    TCPServer_Listen(cHandler->tcp_server);

    cHandler->client_add = _Callback;

    // Suggestion: Could validate _Callback is not NULL before assignment

    *_ConnectionHandler = cHandler;

    return 0;
}

/**
 * @brief Called internally when a new TCP connection is accepted.
 *
 * Initializes a Connection struct and calls the user-provided callback.
 *
 * @param _Context Pointer to ConnectionHandler.
 * @param _Socket File descriptor for the accepted socket.
 * @return 0 on success, negative on error.
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
 * @brief Dispose a ConnectionHandler instance and free all resources.
 *
 * @param _ConnectionHandler Pointer to pointer to ConnectionHandler to dispose.
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