/**
 * @file ConnectionHandler.h
 * @brief Public API for the ConnectionHandler module.
 *
 * Provides the TCP connection handler used to accept new clients and forward
 * them to a registered callback.
 *
 * @defgroup ConnectionHandler ConnectionHandler
 * @ingroup Server
 * @{
 */

#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "../../Libs/Utils/smw.h"
#include "../TCPServer.h"
#include "Connection.h"

/**
 * @brief Callback type for accepted client connections.
 *
 * @param _Connection Pointer to the new Connection.
 *
 * @return 0 on success, negative on error.
 */
typedef int (*Callback)(Connection* _Connection);

/**
 * @brief Connection handler state.
 *
 * Stores the underlying TCP server and the callback used for new clients.
 */
typedef struct{
    TCPServer* tcp_server;   /**< Underlying TCPServer instance. */
    Callback client_add;     /**< Callback for accepted connections. */
} ConnectionHandler;

/**
 * @brief Initializes a ConnectionHandler and starts listening on the port.
 *
 * @param _ConnectionHandler Receives the allocated handler instance.
 * @param _Port TCP port to listen on.
 * @param _Callback Callback used for newly accepted connections.
 *
 * @return 0 on success, negative value on error.
 */
int ConnectionHandler_Initialize(ConnectionHandler **_ConnectionHandler, int _Port, Callback _Callback);

/**
 * @brief Disposes a ConnectionHandler instance.
 *
 * @param _ConnectionHandler Pointer to the handler pointer to dispose.
 */
void ConnectionHandler_Dispose(ConnectionHandler** _ConnectionHandler);

/** @} */

#endif /* CONNECTIONHANDLER_H */
