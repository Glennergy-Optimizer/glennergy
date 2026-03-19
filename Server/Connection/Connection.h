/**
 * @file Connection.h
 * @brief Provides a wrapper for managing TCP client connections.
 * 
 * Includes initialization, handling, and disposal of client connections.
 * Handles reading HTTP requests and sending JSON responses.
 * 
 * @author YourName
 * @date 2026-03-19
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>
#include <semaphore.h>
#include "../../Libs/Utils/smw.h"

/**
 * @brief Represents a client connection over a socket.
 */
typedef struct {
    int socket;             /**< Socket file descriptor */
    uint64_t timeout;       /**< Monotonic timeout timestamp in ms */
    int bytesReadOut;       /**< Number of bytes read from socket */
} Connection;

/**
 * @brief Allocate and initialize a Connection structure.
 *
 * @param _Connection Pointer to a Connection* which will point to the allocated Connection
 * @param _Socket Socket file descriptor for the client connection
 * @return 0 on success, -1 if allocation fails
 */
int Connection_Initialize(Connection** _Connection, int _Socket);

/**
 * @brief Handle incoming HTTP request on the connection and send response.
 *
 * Processes GET requests, parses headers, handles favicon/empty requests,
 * and returns JSON response based on shared memory data.
 *
 * @param _Connection Pointer to initialized Connection
 * @return 0 on success, negative value on error
 */
int Connection_Handle(Connection* _Connection);

/**
 * @brief Dispose a Connection and free resources.
 *
 * Closes the socket and frees memory.
 *
 * @param _Connection Pointer to a Connection* to dispose
 */
void Connection_Dispose(Connection** _Connection);

#endif /* CONNECTION_H */