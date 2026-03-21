/**
 * @file Sockets.h
 * @brief Wrapper functions for UNIX domain socket operations.
 * 
 * Provides simplified APIs for creating, binding, listening,
 * accepting, and connecting UNIX domain sockets.
 * Includes placeholders for socket options.
 * 
 * @author YourName
 * @date 2026-03-19
 */

#ifndef _SOCKETS_H
#define _SOCKETS_H

#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>

/**
 * @brief Create a UNIX domain socket.
 *
 * @return Socket file descriptor on success, -1 on failure.
 */
int socket_CreateSocket();

/**
 * @brief Bind a socket to a specific file system path.
 *
 * @param socket_fd File descriptor of the socket to bind.
 * @param socket_path Path to bind the socket to. Must not be NULL.
 * @return 0 on success, -1 on failure.
 */
int socket_Bind(int socket_fd, const char *socket_path);

/**
 * @brief Mark the socket as a passive socket to accept incoming connections.
 *
 * @param socket_fd Socket file descriptor.
 * @param backlog Maximum length of the queue of pending connections.
 * @return 0 on success, -1 on failure.
 */
int socket_Listen(int socket_fd, int backlog);

/**
 * @brief Accept an incoming client connection.
 *
 * @param socket_fd Socket file descriptor on which to accept a connection.
 * @return Client socket file descriptor on success, -1 on failure.
 */
int socket_Accept(int socket_fd);

/**
 * @brief Connect to a UNIX domain socket at a given path.
 *
 * @param socket_path Path of the socket to connect to. Must not be NULL.
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

#endif /* _SOCKETS_H */