#define MODULE_NAME "TCPServer"
#include "TCPServer.h"
#include "Log/Logger.h"
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void TCPServer_Work(void *_Context, uint64_t monTime);

int TCPServer_Initialize(TCPServer **_TCPServer, int port, int backlog, TCPServer_OnConnection callback, void *context)
{
    TCPServer *tcp_server = (TCPServer *)malloc(sizeof(TCPServer));

    if (tcp_server == NULL)
        return -1;

    tcp_server->onConnect = callback;
    tcp_server->context = context;
    tcp_server->port = port;
    tcp_server->backlog = backlog;
    tcp_server->server_socket = -1;

    *_TCPServer = tcp_server;

    return 0;
}

int TCPServer_Listen(TCPServer *_TCPServer)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    char port_str[10];
    int sock = -1;
    int optval = 1;

    memset(&hints, 0, sizeof(hints));
    snprintf(port_str, sizeof(port_str), "%d", _TCPServer->port);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port_str, &hints, &res) != 0)
    {
        LOG_ERROR("Failed to convert hostname");
        return -1;
    }

    for (struct addrinfo *temp = res; temp; temp = temp->ai_next)
    {
        sock = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);

        if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) <
            0)
        {
            continue;
        }

        if ((bind(sock, temp->ai_addr, temp->ai_addrlen)) < 0)
        {
            LOG_ERROR("Failed to bind socket to port");
            close(sock);
            sock = -1;
            continue;
        }
        else
        {
            break;
        }
    }

    freeaddrinfo(res);

    if (listen(sock, _TCPServer->backlog) < 0)
    {
        close(sock);
        return -2;
    }

    _TCPServer->server_socket = sock;

    int flags = fcntl(_TCPServer->server_socket, F_GETFL, 0);
    fcntl(_TCPServer->server_socket, F_SETFL, flags | O_NONBLOCK);

    _TCPServer->task = smw_create_task(_TCPServer, TCPServer_Work);
    LOG_INFO("TCP Server listening");
    return 0;
}

int TCPServer_Accept(TCPServer *_TCPServer)
{
    int sock = accept(_TCPServer->server_socket, NULL, NULL);

    if (sock < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return -1;

        perror("accept");
        return -1;
    }

    int result = _TCPServer->onConnect(_TCPServer->context, sock);

    if (result < 0)
    {
        LOG_WARNING("Connection callback failed");
        close(sock);
        return -1;
    }

    return 0;
}

void TCPServer_Work(void *_Context, uint64_t monTime)
{
    TCPServer* tcp_server = (TCPServer *)_Context;

    if (tcp_server == NULL)
        return;

    TCPServer_Accept(tcp_server);
}

void TCPServer_Disconnect(int socket)
{

    close(socket);
    socket = -1;
}

void TCPServer_Dispose(TCPServer** _TCPServer)
{
    if (_TCPServer == NULL || *_TCPServer == NULL)
        return;

    TCPServer* tcp_server = *_TCPServer;

    if (tcp_server->task != NULL)
        smw_destroy_task(tcp_server->task);

    close(tcp_server->server_socket);

   // free(_TCPServer->client);
    //_TCPServer->client = NULL;

    free(tcp_server);
    tcp_server = NULL;
}
/*#include "../Libs/tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

// Forward declarations
void* connection_Thread(void *arg);

int server_Init(ServerConfig *config)
{
    if (!config) {
        return -1;
    }
    config->running = 0;
    return 0;
}

void server_Start(ServerConfig *config)
{
    int server_socket;
    struct sockaddr_in connection_addr;
    
    server_socket = TCP_CreateSocket();
    if (server_socket < 0)
    {
        return;
    }
    
    if (TCP_SetSocketOptions(server_socket) < 0)
    {
        close(server_socket);
        return;
    }
    
    if (TCP_Bind(server_socket, config->port) < 0)
    {
        close(server_socket);
        return;
    }
    
    if (TCP_Listen(server_socket, config->backlog) < 0) {
        close(server_socket);
        return;
    }
    
    printf("Server listening on port %d...\n", config->port);
    config->running = 1;
    
    while (config->running)
    {
        int *connection_socket = malloc(sizeof(int));
        *connection_socket = TCP_Accept(server_socket, &connection_addr);
        
        if (*connection_socket < 0)
        {
            perror("Accept failed");
            free(connection_socket);
            continue;
        }
        
        printf("Client connected: %s:%d\n", inet_ntoa(connection_addr.sin_addr), ntohs(connection_addr.sin_port));
        
        // Create thread to handle connection
        pthread_t thread;
        if (pthread_create(&thread, NULL, connection_Thread, connection_socket) != 0)
        {
            perror("Thread creation failed");
            close(*connection_socket);
            free(connection_socket);
        } 
        else
        {
            pthread_detach(thread);
        }
    }
    
    close(server_socket);
}

// Thread function to handle connections
void* connection_Thread(void *socket_fd)
{
    int connection_socket = *(int *)socket_fd;
    free(socket_fd);
    handle_Connection(connection_socket);
    return NULL;
}

void handle_Connection(int connection_socket)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received < 0)
    {
        perror("recv failed");
        close(connection_socket);
        return;
    }
    
    buffer[bytes_received] = '\0';
    printf("Received request:\n%s\n", buffer);
    
    const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    send(connection_socket, response, strlen(response), 0);
    
    close(connection_socket);
}


void server_Stop(ServerConfig *config)
{
    if (config) {
        config->running = 0;
    }
}*/
