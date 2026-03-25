/**
 * @file Connection.c
 * @brief Implementation of TCP client connection management.
 * @ingroup Connection
 *
 * Handles initialization, reading HTTP requests, sending JSON responses,
 * and resource cleanup for Connection structures.
 */

#define MODULE_NAME "Connection"

#include "Connection.h"
#include "../TCPServer.h"
#include "../Log/Logger.h"
#include "../../Algorithm/AlgoritmProtocol.h"
#include "../../Libs/SHM.h"
#include "../HTTP/HTTPRequest.h"
#include "../../Libs/Utils/utils.h"
#include <jansson.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h> // Include semaphore so we dont get warnings

#define RESPONSE_HEADER "HTTP/1.1 200 OK\r\n"                            \
                        "Content-Length: %zu\r\n"                        \
                        "Content-Type: application/json\r\n"             \
                        "Access-Control-Allow-Origin: *\r\n"             \
                        "Access-Control-Allow-Methods: GET, OPTIONS\r\n" \
                        "Access-Control-Allow-Headers: Content-Type\r\n" \
                        "Connection: close\r\n"                          \
                        "\r\n"                                           \
                        "%s"

void Connection_Work(void *_Context, uint64_t monTime);

void Connection_Dispose(Connection **_Connection);

int Connection_Initialize(Connection **_Connection, int _Socket)
{
    Connection *connection = (Connection *)malloc(sizeof(Connection));
    if (connection == NULL)
        return -1;

    connection->socket = _Socket;
    connection->timeout = 0;
    // Behöver vi "connection->bytesReadOut = 0;" här? Det sätts ju i Connection_Handle så kanske inte nödvändigt att initiera det här?

    *_Connection = connection;
    return 0;
}

int Connection_Handle(Connection *_Connection)
{
    LOG_INFO("Handling incoming connection");
    LOG_DEBUG("Sending response to socket");

    HTTPRequest request;
    int result = 9999;
    char *json_data;

    HTTPRequest_Initialize(&request);

    while (result != Connection_ReadResult_Success)
    {
        uint64_t monTime = SystemMonotonicMS();
        result = HTTPRequest_ReadHeaders(_Connection->socket, &request, &_Connection->bytesReadOut);

        if (_Connection->timeout > 0)
        {
            if (monTime >= _Connection->timeout)
        {
            LOG_INFO("Client timed out");
            HTTPRequest_Dispose(&request);
            return -1;
        }
        }
        else
        {
            _Connection->timeout = monTime + 3000;
        }

        if (_Connection->bytesReadOut > 0)
        {
            _Connection->timeout = 0;
        }
    }

    HTTPRequest_ParseHeader(&request);

    printf("Request: %s\n", request.url ? request.url : "NULL");
    // Browsers can automatically add a second get request with favicon.ico, which can cause issues if we try to parse it as an integer.
    // If we have a request for favicon, just ignore it
    if (request.url != NULL && strcmp(request.url, "/favicon.ico") == 0)
    {
        const char *resp = "HTTP/1.1 204 No Content\r\n"
                           "Content-Length: 0\r\n"
                           "Connection: close\r\n"
                           "\r\n";
        send(_Connection->socket, resp, strlen(resp), MSG_NOSIGNAL);
        HTTPRequest_Dispose(&request);
        return 0;
    }

    // Browsers also sometimes sends an empty "/" request so let's handle that too.
    if (request.url == NULL || strcmp(request.url, "/") == 0)
    {
        const char *resp = "HTTP/1.1 204 No Content\r\n"
                           "Content-Length: 0\r\n"
                           "Connection: close\r\n"
                           "\r\n";
        send(_Connection->socket, resp, strlen(resp), MSG_NOSIGNAL);
        HTTPRequest_Dispose(&request);
        return 0;
    }
    // Now, if we have a get request we actually want to handle, i.e "/id=3", we continue handling it

    // OBS/TODO - I produktion måste den här vara aktiv för att HTTP requesten ska fungera direkt och få data
    char *rec_offset = request.url + 1;
    int client_id = strtol(request.url, &rec_offset, 10);
    // Men den här behövs för att Håkan ska kunna kompilera. Raden ovanför verkar funka med ubuntu
    // int client_id = strtol(request.url + 1, NULL, 10);
    printf("id: %d\n", client_id);

    int shm_fd = -1;
    sem_t *mutex;
    AlgoritmShared *memory;

    if (SHM_InitializeReader(&memory, ALGORITM_SHARED, shm_fd) != 0)
        return -1;

    if (SHM_OpenSemaphore(&mutex, ALGORITM_MUTEX) != 0)
        return -2;

    sem_wait(mutex);

    json_t *arr = json_array();
    for (int i = 0; i < MAX_ID; i++)
    {
        if (client_id != memory->result[i].id)
            continue;

        printf("Got the stuff: %d\n", memory->result[i].id);
        for (int j = 0; j < 96; j++)
        {
<<<<<<< HEAD

            printf("Recommendation: %.3f\n", memory->result[i].recommendation[j]);
            double rec = memory->result[i].recommendation[j];
=======
            printf("Recommendation: %d\n", memory->result[i].recommendation[j]);
            int rec = memory->result[i].recommendation[j];
            const char *type = NULL;
>>>>>>> 134fa1332d9b65fe00fc1e597cc285c0c1c4f593

            const char *type = NULL;

            json_t *obj = json_object();
            json_object_set_new(obj, "id", json_integer(memory->result[i].id));
<<<<<<< HEAD

            json_object_set_new(obj, "type", json_real(rec));

=======
            json_object_set_new(obj, "type", json_string(type));
>>>>>>> 134fa1332d9b65fe00fc1e597cc285c0c1c4f593
            json_object_set_new(obj, "timestamp", json_string(memory->result[i].time[j].time));
            json_array_append_new(arr, obj);

            if (strstr(memory->result[i].time[j].time, "23:45") != NULL)
            {
                break;
            }
        }
    }

    json_data = json_dumps(arr, JSON_INDENT(4));
    // printf("data %s\n", json_data);
    json_decref(arr);
    sem_post(mutex);

    // snprintf(RESPONSE_HEADER, sizeof(RESPONSE_HEADER), "%s", json_data);

    printf("size of json data: %zu\n", strlen(json_data));

    char response[12000];
    // snprintf(response, sizeof(response), RESPONSE_HEADER, strlen(json_data), json_data);
    int actualLength = snprintf(response, sizeof(response), RESPONSE_HEADER, strlen(json_data), json_data);
    if (actualLength >= sizeof(response))
    {
        LOG_ERROR("Response truncated: actual length %d exceeds buffer size %zu", actualLength, sizeof(response));
        return -1; // TODO - Free här vid error?
    }

    printf("data %s\n", response);
    send(_Connection->socket, response, actualLength, MSG_NOSIGNAL);

    free(json_data); // Don't forget to free the data after we sent the response
    LOG_DEBUG("Response sent");

    SHM_CloseSemaphore(&mutex);
    SHM_DisposeReader(&memory, ALGORITM_SHARED, shm_fd);
    HTTPRequest_Dispose(&request);

    LOG_DEBUG("Response sent");
    return 0;
}

void Connection_Dispose(Connection **_Connection)
{
    if (_Connection == NULL || *_Connection == NULL)
        return;

    Connection *connection = *_Connection;

    close(connection->socket);
    free(connection);
    connection = NULL; // Todo 
}