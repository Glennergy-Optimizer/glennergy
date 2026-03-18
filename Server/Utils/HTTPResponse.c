#include "HTTPResponse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define RESPONSE_HEADER "HTTP/1.1 %d %s\r\n"                                  \
                        "Content-Length: %zu\r\n"                             \
                        "Content-Type: application/json\r\n"                  \
                        "Access-Control-Allow-Origin: *\r\n"                  \
                        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n" \
                        "Access-Control-Allow-Headers: Content-Type\r\n"      \
                        "\r\n"                                                \
                        "%s"

int HTTPResponse_Initialize(HTTPResponse *http_response)
{
    if (http_response == NULL)
        return -1;

    http_response->status_code = 200;
    http_response->response_length = 0;
    http_response->response = NULL;
    http_response->response_formatted = NULL;

    return 0;
}

int HTTPResponse_Format(HTTPResponse *http_response)
{
    if (http_response == NULL)
        return -1;

    if (http_response->response == NULL)
        return -2;

    const char *status_text;
    switch (http_response->status_code)
    {
    case 200:
        status_text = "OK";
        break;
    case 201:
        status_text = "Created";
        break;
    case 400:
        status_text = "Bad Request";
        break;
    case 404:
        status_text = "Not Found";
        break;
    case 408:
        status_text = "Connection Timeout";
        break;
    case 500:
        status_text = "Internal Server Error";
        break;
    default:
        status_text = "Unknown";
        break;
    }

    char response[2048];
    http_response->response_length = snprintf(response, sizeof(response), RESPONSE_HEADER,
                                              http_response->status_code, status_text, strlen(http_response->response), http_response->response);

    http_response->response_formatted = strdup(response);

    if (http_response->response_formatted == NULL)
        return -1;

    return 0;
}

void HTTPResponse_Dispose(HTTPResponse *http_response)
{
    if (http_response == NULL)
        return;

    if (http_response->response != NULL)
    {
        free(http_response->response);
        http_response->response = NULL;
    }

    if (http_response->response_formatted != NULL)
    {
        free(http_response->response_formatted);
        http_response->response_formatted = NULL;
    }
}