#define MODULE_NAME "Connection"
#include "Connection.h"
#include "../TCPServer.h"
#include "../Log/Logger.h"
#include "../../Algorithm/testreader.h"
#include "../../Libs/SHM.h"
#include "../HTTP/HTTPRequest.h"
#include "../../Libs/Utils/utils.h"
#include <jansson.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>

#define RESPONSE_HEADER "HTTP/1.1 200 OK\r\n"                            \
                        "Content-Length: %zu\r\n"                        \
                        "Content-Type: application/json\r\n"             \
                        "Access-Control-Allow-Origin: *\r\n"             \
                        "Access-Control-Allow-Methods: GET, OPTIONS\r\n" \
                        "Access-Control-Allow-Headers: Content-Type\r\n" \
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
                // break;
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

    printf("Request: %s\n", request.url);

    int client_id = strtol(request.url, request.url + 1, 10);
    printf("id: %d\n", client_id);

    int shm_fd;
    sem_t *mutex;

    SharedMemory *memory;

    if (SHM_InitializeReader(&memory, ALGORITM_SHARED, shm_fd) != 0)
        return -1;

    if (SHM_OpenSemaphore(&mutex, ALGORITM_MUTEX) != 0)
        return -2;

    sem_wait(mutex);

    json_t *arr = json_array();

    for (int i = 0; i < MAX_ID; i++)
    {
        if (client_id != memory->result[i].id)
        {
            continue;
        }

        printf("Got the stuff: %d\n", memory->result[i].id);
        for (int j = 0; j < 96; j++)
        {
            int rec = memory->result[i].recommendation[j];
            const char *type = NULL;

            if (rec == 1)
                type = "BUY";
            else if (rec == 2)
                type = "HOLD";
            else if (rec == 3)
                type = "SELL";
            else
                continue;

            json_t *obj = json_object();

            json_object_set_new(obj, "id", json_integer(memory->result[i].id));

            json_object_set_new(obj, "type", json_string(type));

            json_object_set_new(obj, "timestamp", json_string(memory->result[i].time[j].time));

            json_array_append_new(arr, obj);
        }
    }
    json_data = json_dumps(arr, JSON_INDENT(4));
    // printf("data %s\n", json_data);
    json_decref(arr);
    sem_post(mutex);

    //snprintf(RESPONSE_HEADER, sizeof(RESPONSE_HEADER), "%s", json_data);

    char response[8192];
    //snprintf(response, sizeof(response), RESPONSE_HEADER, strlen(json_data), json_data);
    int actualLength = snprintf(response, sizeof(response), RESPONSE_HEADER, strlen(json_data), json_data);
    if (actualLength >= sizeof(response))
    {
        LOG_ERROR("Response truncated: actual length %d exceeds buffer size %zu", actualLength, sizeof(response));
        return -1;
    }


    printf("data \n", response);
    send(_Connection->socket, response, actualLength, MSG_NOSIGNAL);
    LOG_DEBUG("Response sent");

    SHM_CloseSemaphore(&mutex);

    SHM_DisposeReader(&memory, ALGORITM_SHARED, shm_fd);
    HTTPRequest_Dispose(&request);
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