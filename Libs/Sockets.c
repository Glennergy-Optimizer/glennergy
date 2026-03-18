#include "Sockets.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int socket_CreateSocket()
{
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }
    return socket_fd;
}

// int socket_SetSocketOptions(int socket_fd)
// {
//     int opt = 1;
//     if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
//         perror("setsockopt failed");
//         return -1;
//     }
//     return 0;
// }

int socket_Bind(int socket_fd, const char *socket_path)
{
    if (socket_path == NULL) {
        fprintf(stderr, "Socket path cannot be NULL\n");
        return -1;
    }
    //unlink(socket_path);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    
    if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return -1;
    }
    
    return 0;
}

int socket_Listen(int socket_fd, int backlog)
    {
    if (listen(socket_fd, backlog) < 0)
    {
        perror("Listen failed");
        return -1;
    }
    return 0;
}

int socket_Accept(int socket_fd)
{
    int client_fd = accept(socket_fd, NULL, NULL);
    
    if (client_fd < 0) {
        perror("Accept failed");
        return -1;
    }
    
    return client_fd;
}

int socket_Connect(const char *socket_path)
{
    if (socket_path == NULL) {
        fprintf(stderr, "Socket path cannot be NULL\n");
        return -1;
    }

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Connect failed");
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}