/**
 * @file Connection.c
 * @brief Implementation of TCP client connection management.
 *
 * @ingroup Connection
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

/**
 * @brief Allocates and initializes a Connection instance.
 */
int Connection_Initialize(Connection **_Connection, int _Socket)
{
    Connection *connection = (Connection *)malloc(sizeof(Connection));
    if (connection == NULL)
        return -1;

    connection->socket = _Socket;
    connection->timeout = 0;
    connection->bytesReadOut = 0;
    // Behöver vi "connection->bytesReadOut = 0;" här? Det sätts ju i Connection_Handle så kanske inte nödvändigt att initiera det här?

    *_Connection = connection;
    return 0;
}

/**
 * @brief Handles an incoming client connection and sends a response.
 *
 * See header for full contract documentation.
 */
int Connection_Handle(Connection *_Connection)
{
    LOG_INFO("Handling incoming connection");
    LOG_DEBUG("Sending response to socket");

    HTTPRequest request;
    int result = 9999;
    char *json_data = NULL;

    // Init stuff all stuff here at start of function
    sem_t *mutex = NULL;
    AlgoritmShared *memory = NULL;
    // And add status
    int status = 0;


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

    // If parsed failed, also stop and dispose the request 
    if (HTTPRequest_ParseHeader(&request) != 0)
    {
        const char *resp = "HTTP/1.1 400 Bad Request\r\n"
                           "Content-Length: 0\r\n"
                           "Connection: close\r\n"
                           "\r\n";
        send(_Connection->socket, resp, strlen(resp), MSG_NOSIGNAL);
        HTTPRequest_Dispose(&request);
        return -1;
    }
    //HTTPRequest_ParseHeader(&request);

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
    HTTPRequestData request_data;
    if (parse_request(request.url, &request_data) != 0) {
        const char *resp =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";

        send(
            _Connection->socket,
            resp,
            strlen(resp),
            MSG_NOSIGNAL
        );

        HTTPRequest_Dispose(&request);
        return -1;
    }

    printf("Parsed request ID: %d Command: %s\n", request_data.id, request_data.command);

    //HTTPRequestData request_data = parse_request(request.url);
    // printf("Parsed request ID: %d Command: %s\n", request_data.id, request_data.command);
    // char *rec_offset = request.url + 1;
    // int client_id = strtol(request.url, &rec_offset, 10);
    // // Men den här behövs för att Håkan ska kunna kompilera. Raden ovanför verkar funka med ubuntu
    // // int client_id = strtol(request.url + 1, NULL, 10);
    //printf("id: %d\n", client_id); 

    //int shm_fd = -1;
    // sem_t *mutex;
    // AlgoritmShared *memory;

    //if (SHM_InitializeReader(&memory, ALGORITM_SHARED, shm_fd) != 0)
    // if (SHM_InitializeReader(&memory, ALGORITM_SHARED) != 0)
    //     return -1;
    if (SHM_InitializeReader(&memory, ALGORITM_SHARED) != 0)
    { 
        status = -1;
        goto cleanup;
    }
    // if (SHM_OpenSemaphore(&mutex, ALGORITM_MUTEX) != 0)
    //     return -2;
    if (SHM_OpenSemaphore(&mutex, ALGORITM_MUTEX) != 0)
    {
        status = -2;
        goto cleanup;
    }

    sem_wait(mutex);

    json_t *arr = json_array();
    for (int i = 0; i < MAX_ID; i++)
    {
        if (request_data.id != memory->result[i].id)
            continue;

        printf("Got the stuff: %d\n", memory->result[i].id);

        if (strncmp(request_data.command, "recommendation", 15) == 0)
        {
            size_t result_count = memory->result[i].count;
            if (result_count > MAX_FORECAST_ENTRIES)
                result_count = MAX_FORECAST_ENTRIES;
            for (size_t j = 0; j < result_count; j++)
            {

                // printf("Recommendation: %.f\n", memory->result[i].sample[j]);
                double rec = memory->result[i].recommendation[j];
                const char *recommendation = "unknown";
                switch (memory->result[i].recommendation_type[j])
                {
                case 1:
                    recommendation = "buy";
                    break;
                case 2:
                    recommendation = "hold";
                    break;
                case 3:
                    recommendation = "sell";
                    break;
                }

                json_t *obj = json_object();
                json_object_set_new(obj, "id", json_integer(memory->result[i].id));
                json_object_set_new(obj, "score", json_real(rec));
                json_object_set_new(obj, "recommendation", json_string(recommendation));
                json_object_set_new(obj, "timestamp", json_string(memory->result[i].time[j].time));
                json_array_append_new(arr, obj);

                // if (strstr(memory->result[i].time[j].time, "23:45") != NULL)
                //{
                //     break;
                //}
            }
        }
        else if (strncmp(request_data.command, "weather", 8) == 0)
        {
            size_t result_count = memory->result[i].count;
            if (result_count > MAX_FORECAST_ENTRIES)
                result_count = MAX_FORECAST_ENTRIES;
            for (size_t j = 0; j < result_count; j++)
            {

                // printf("Recommendation: %.f\n", memory->result[i].sample[j]);
                double rec = memory->result[i].recommendation[j];

                json_t *obj = json_object();
                json_object_set_new(obj, "timestamp", json_string(memory->result[i].time[j].time));
                json_object_set_new(obj, "temp", json_real(memory->result[i].weather.temp[j]));
                json_object_set_new(obj, "weather_code", json_integer(memory->result[i].weather.weather_code[j]));
                json_object_set_new(obj, "uv_index", json_real(memory->result[i].weather.uv_index[j]));
                json_array_append_new(arr, obj);

                // if (strstr(memory->result[i].time[j].time, "23:45") != NULL)
                //{
                //     break;
                //}
            }
        }
        else if (strncmp(request_data.command, "price", 6) == 0)
        {
            size_t result_count = memory->result[i].count;
            if (result_count > MAX_FORECAST_ENTRIES)
                result_count = MAX_FORECAST_ENTRIES;
            for (size_t j = 0; j < result_count; j++)
            {

                // printf("Recommendation: %.f\n", memory->result[i].sample[j]);
                double rec = memory->result[i].recommendation[j];

                json_t *obj = json_object();
                json_object_set_new(obj, "timestamp", json_string(memory->result[i].time[j].time));
                json_object_set_new(obj, "price_sek_per_kwh", json_real(memory->result[i].price[j]));
                json_array_append_new(arr, obj);

                // if (strstr(memory->result[i].time[j].time, "23:45") != NULL)
                //{
                //     break;
                //}
            }
        }
    }

    json_data = json_dumps(arr, JSON_INDENT(4));
    // printf("data %s\n", json_data);
    json_decref(arr);
    sem_post(mutex);

    // snprintf(RESPONSE_HEADER, sizeof(RESPONSE_HEADER), "%s", json_data);

    printf("size of json data: %zu\n", strlen(json_data));

    char response[30000];
    // snprintf(response, sizeof(response), RESPONSE_HEADER, strlen(json_data), json_data);
    int actualLength = snprintf(response, sizeof(response), RESPONSE_HEADER, strlen(json_data), json_data);
    if (actualLength >= sizeof(response))
    {
        LOG_ERROR("Response truncated: actual length %d exceeds buffer size %zu", actualLength, sizeof(response));
        //return -1; // TODO - Free här vid error?
        status = -1;
        goto cleanup;
    }

    status = 0;
    
    
    //printf("data %s\n", response);

    send(_Connection->socket, response, actualLength, MSG_NOSIGNAL);
    
    //free(json_data); // Don't forget to free the data after we sent the response
    LOG_DEBUG("Response sent");
        // SHM_CloseSemaphore(&mutex);
    // //SHM_DisposeReader(&memory, ALGORITM_SHARED, shm_fd);
    // SHM_DisposeReader(&memory, ALGORITM_SHARED);
    // HTTPRequest_Dispose(&request);
    cleanup:
        free(json_data);
    
        if (mutex != NULL)
        {
            SHM_CloseSemaphore(&mutex);
        }
        if (memory != NULL)
        {
            SHM_DisposeReader(&memory);
        }
        HTTPRequest_Dispose(&request);
    printf("End of handler function!!!!!");
    fflush(stdout);
    LOG_DEBUG("Response sent");
    return status;
}

/**
 * @brief Releases a Connection and its socket.
 */
void Connection_Dispose(Connection **_Connection)
{
    if (_Connection == NULL || *_Connection == NULL)
        return;

    Connection *connection = *_Connection;

    close(connection->socket);
    printf("client disconnected!");
    free(connection);
    connection = NULL; // Todo
}
