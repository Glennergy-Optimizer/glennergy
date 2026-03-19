/**
 * @file ConnectionHandler.h
 * @brief API for handling multiple TCP connections via ConnectionHandler.
 *
 * Provides initialization, disposal, and callback mechanisms for managing connections.
 *
 * @author YourName
 * @date 2026-03-19
 */

#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "../../Libs/Utils/smw.h"
#include "../TCPServer.h"
#include "Connection.h"

/**
 * @brief Callback type for new client connections.
 * @param _Connection Pointer to the new Connection.
 * @return int Status code: 0 on success, negative on error.
 */
typedef int (*Callback)(Connection* _Connection);

/**
 * @brief ConnectionHandler struct managing TCP server and callbacks.
 */
typedef struct{
    TCPServer* tcp_server;   /**< Pointer to underlying TCPServer instance */
    Callback client_add;     /**< Callback to handle newly accepted connections */
} ConnectionHandler;

/**
 * @brief Initialize a ConnectionHandler and start listening on the given port.
 *
 * @param _ConnectionHandler Pointer to pointer of ConnectionHandler to initialize.
 * @param _Port TCP port to listen on.
 * @param _Callback Callback function for handling new connections.
 * @return 0 on success, negative value on error.
 * @pre _ConnectionHandler must not be NULL.
 * @post Allocates and initializes a ConnectionHandler instance.
 */
int ConnectionHandler_Initialize(ConnectionHandler **_ConnectionHandler, int _Port, Callback _Callback);

/**
 * @brief Dispose a ConnectionHandler instance.
 *
 * @param _ConnectionHandler Pointer to pointer of ConnectionHandler to dispose.
 * @post Frees all resources associated with the handler.
 */
void ConnectionHandler_Dispose(ConnectionHandler** _ConnectionHandler);

#endif /* CONNECTIONHANDLER_H */