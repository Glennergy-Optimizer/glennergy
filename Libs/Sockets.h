/**
 * @file Sockets.h
 * @brief Public API for UNIX domain socket wrapper functions.
 *
 * Provides simplified helpers for creating, binding, listening, accepting,
 * and connecting UNIX domain sockets.
 *
 * @defgroup Sockets Sockets
 * @brief UNIX domain socket wrapper functions.
 *
 * Thin wrappers around common UNIX domain socket operations used by the
 * repository.
 * @{
 */

#ifndef _SOCKETS_H
#define _SOCKETS_H

#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>

/**
 * @brief Creates a UNIX domain socket.
 *
 * @return Socket file descriptor on success, -1 on failure.
 */
int socket_CreateSocket();

/**
 * @brief Binds a socket to a specific file system path.
 *
 * @param socket_fd File descriptor of the socket to bind.
 * @param socket_path Path to bind the socket to. Must not be NULL.
 *
 * @return 0 on success, -1 on failure.
 */
int socket_Bind(int socket_fd, const char *socket_path);

/**
 * @brief Marks a socket as passive so it can accept incoming connections.
 *
 * @param socket_fd Socket file descriptor.
 * @param backlog Maximum length of the queue of pending connections.
 *
 * @return 0 on success, -1 on failure.
 */
int socket_Listen(int socket_fd, int backlog);

/**
 * @brief Accepts an incoming client connection.
 *
 * @param socket_fd Socket file descriptor on which to accept a connection.
 *
 * @return Client socket file descriptor on success, -1 on failure.
 */
int socket_Accept(int socket_fd);

/**
 * @brief Connects to a UNIX domain socket at a given path.
 *
 * @param socket_path Path of the socket to connect to. Must not be NULL.
 *
 * @return Socket file descriptor on success, -1 on failure.
 */
int socket_Connect(const char *socket_path);

/**
 * @brief Set options on a socket (e.g., SO_REUSEADDR).
 *
 * This function is currently a placeholder and commented out.
 *
 * @param socket_fd Socket file descriptor to set options on.
 * @return 0 on success, -1 on failure.
 */
// int socket_SetSocketOptions(int socket_fd);

/** @} */

#endif /* _SOCKETS_H */
