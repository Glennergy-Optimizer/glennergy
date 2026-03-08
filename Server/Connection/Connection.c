#define MODULE_NAME "Connection"
#include "Connection.h"
#include "../TCPServer.h"
#include "../Log/Logger.h"
#include "../Utils/APIHandler.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>


#define RESPONSE_HEADER "HTTP/1.1 200 OK\r\n"                            \
                        "Content-Length: 100\r\n"                        \
                        "Content-Type: application/json\r\n"             \
                        "Access-Control-Allow-Origin: *\r\n"             \
                        "Access-Control-Allow-Methods: GET, OPTIONS\r\n" \
                        "Access-Control-Allow-Headers: Content-Type\r\n" \
                        "\r\n"                                           \
                        "Hello from server!\r\n"

void Connection_Work(void *_Context, uint64_t monTime);

void Connection_Dispose(Connection **_Connection);

int Connection_Initialize(Connection **_Connection, int _Socket, APIHandler_t *api_ctx)
{
    Connection *connection = (Connection *)malloc(sizeof(Connection));

    if (connection == NULL)
        return -1;

    connection->socket = _Socket;
    connection->api_ctx = api_ctx;
    *_Connection = connection;
    return 0;
}

int Connection_Handle(Connection *_Connection)
{
    LOG_INFO("Handling incoming connection");
    char buffer[4096];
    ssize_t bytes_read = recv(_Connection->socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        LOG_ERROR("Failed to read from socket");
        return -1;
    }
    buffer[bytes_read] = '\0';

    HTTPRequest_t req = {0};
    if (HTTPRequest_ParseHeaders(&req, buffer, bytes_read) < 0) { 
        LOG_ERROR("Failed to parse HTTP request");
        return -1;
    }

    HTTPResponse_t resp = {0};
    if (APIHandler_HandleRequest(_Connection->api_ctx, &req, &resp) < 0) {
        LOG_ERROR("Failed to handle API request");
        if (resp.body == NULL) {
            resp.status_code = 500;
            resp.body = strdup("{\"error\":\"Internal server error\"}");
        }
    }
    LOG_DEBUG("Sending response to socket");
    if (HTTPResponse_Format(&resp) == 0) {
        send(_Connection->socket, resp.resp_formatted, resp.resp_length, MSG_NOSIGNAL);
        LOG_INFO("Response sent");
    } else {
        LOG_ERROR("Failed to format HTTP response");
    }

    HTTPRequest_Dispose(&req);
    HTTPResponse_Dispose(&resp);
    return 0;
}

void Connection_Dispose(Connection **_Connection)
{
    if (_Connection == NULL || *_Connection == NULL)
        return;

    Connection *connection = *_Connection;

    close(connection->socket);

    free(connection);
    connection = NULL;
}