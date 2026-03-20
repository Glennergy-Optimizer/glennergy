/**
 * @file wrapper.h
 * @brief Thin wrapper around socket operations for safer usage.
 * @defgroup Wrapper Wrapper
 * @ingroup Server
 *
 * Provides simplified and consistent interfaces for:
 * - Setting socket timeouts
 * - Receiving data
 * - Sending data
 *
 * @note These functions wrap standard POSIX socket calls.
 * @note Errors are normalized to return -1.
 */

#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdint.h>
#include <stddef.h>

typedef struct timeval timeval;

/**
 * @brief Set receive timeout on a socket.
 *
 * @param socket Socket file descriptor
 * @param seconds Timeout duration in seconds
 * @return 0 on success, -1 on error
 *
 * @pre socket must be a valid socket descriptor
 * @pre seconds >= 0
 * @post Socket will have SO_RCVTIMEO set
 *
 * @note After timeout, recv() returns -1 with errno = EAGAIN/EWOULDBLOCK
 */
int Connection_SetTimeout(int socket, int seconds);

/**
 * @brief Receive data from a socket.
 *
 * @param socket Socket file descriptor
 * @param buffer Buffer to store received data
 * @param length Maximum number of bytes to read
 * @return Number of bytes read, 0 if connection closed, -1 on error
 *
 * @pre buffer must not be NULL
 * @pre length > 0
 * @post Buffer contains received data if return > 0
 *
 * @note This is a thin wrapper around recv()
 */
int Connection_Recv(int socket, uint8_t *buffer, size_t length);

/**
 * @brief Send data over a socket.
 *
 * @param socket Socket file descriptor
 * @param buffer Data to send
 * @param length Number of bytes to send
 * @return Number of bytes written, -1 on error
 *
 * @pre buffer must not be NULL
 * @pre length > 0
 * @post Data is sent over the socket if return >= 0
 *
 * @note Uses MSG_NOSIGNAL to avoid SIGPIPE
 */
int Connection_Send(int socket, const char *buffer, size_t length);

#endif