/**
 * @file Connection.h
 * @brief Public API for TCP client connection handling.
 *
 * @defgroup Connection Connection
 * @ingroup Server
 * @brief TCP client connection lifecycle and request handling.
 *
 * Manages initialization, HTTP request processing, JSON response generation,
 * and cleanup for client connections.
 * @{
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>
#include <semaphore.h>
#include "../../Libs/Utils/smw.h"

/**
 * @brief Represents a client connection over a socket.
 *
 * Memory for this structure is allocated by Connection_Initialize and released
 * by Connection_Dispose.
 */
typedef struct {
    int socket;             /**< Socket file descriptor. */
    uint64_t timeout;       /**< Monotonic timeout timestamp in ms. */
    int bytesReadOut;       /**< Number of bytes read from socket. */
} Connection;

/**
 * @brief Allocates and initializes a Connection structure.
 *
 * @param[out] _Connection Pointer that receives the allocated Connection.
 * @param[in] _Socket Socket file descriptor for the client connection.
 *
 * @return
 * - 0 on success
 * - -1 if allocation fails
 *
 * @pre _Connection must not be NULL.
 * @post *_Connection points to an initialized Connection on success.
 */
int Connection_Initialize(Connection** _Connection, int _Socket);

/**
 * @brief Handles an incoming HTTP request and sends a response.
 *
 * @param[in,out] _Connection Initialized connection to process.
 *
 * @return
 * - 0 on success
 * - negative value on error
 */
int Connection_Handle(Connection* _Connection);

/**
 * @brief Releases a Connection and its resources.
 *
 * @param[in,out] _Connection Pointer to a Connection pointer to dispose.
 */
void Connection_Dispose(Connection** _Connection);

/** @} */

#endif /* CONNECTION_H */
