/**
 * @file wrapper.c
 * @brief Implementation of socket wrapper utilities.
 * @ingroup Wrapper
 *
 * Implements thin wrappers for POSIX socket operations with
 * simplified error handling.
 */

#include "wrapper.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

//After timeout, recv() will return -1 with errno = EAGAIN/EWOULDBLOCK

/**
 * @brief Set receive timeout for a socket.
 *
 * @param socket Socket file descriptor
 * @param seconds Timeout duration in seconds
 * @return 0 on success, -1 on error
 *
 * @pre socket must be valid
 * @pre seconds >= 0
 * @post SO_RCVTIMEO is configured on the socket
 */
int Connection_SetTimeout(int socket, int seconds)
{
    timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        return -1;
    }
    
    return 0;

    // Suggestion: Could validate socket >= 0 before calling setsockopt
}

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
 * @note Wraps recv() and normalizes error handling
 */
int Connection_Recv(int socket, uint8_t *buffer, size_t length)
{
    if (buffer == NULL || length == 0)
        return -1;

    ssize_t bytes_read = recv(socket, buffer, length, 0);
    if (bytes_read < 0)
        return -1;
    if (bytes_read == 0)
        return 0;
    
    return (int)bytes_read;

    // Suggestion: Could handle EINTR retry logic for robustness
}

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
 * @post Data is written to socket if return >= 0
 *
 * @note Uses MSG_NOSIGNAL to prevent SIGPIPE
 */
int Connection_Send(int socket, const char *buffer, size_t length)
{
    if (buffer == NULL || length == 0)
        return -1;

    ssize_t bytes_written = send(socket, buffer, length, MSG_NOSIGNAL);
    if (bytes_written < 0)
        return -1;

    return (int)bytes_written;

    // Suggestion: Could handle partial sends in a loop for full transmission
}