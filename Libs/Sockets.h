#ifndef _SOCKETS_H
#define _SOCKETS_H

#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>

// Create a socket socket
int socket_CreateSocket();

// Bind socket to address and port
int socket_Bind(int socket_fd, const char *socket_path);

// Listen for incoming connections
int socket_Listen(int socket_fd, int backlog);

// Accept a client connection
int socket_Accept(int socket_fd);

int socket_Connect(const char *socket_path);
// // Set socket options (reuse address, etc.)
// int socket_SetSocketOptions(int socket_fd);

#endif
