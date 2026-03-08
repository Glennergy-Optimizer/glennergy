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

int HTTPResponse_Initialize(HTTPResponse_t *resp)
{
    if (resp == NULL)
        return -1;

    resp->status_code = 200;
    resp->resp_length = 0;
    resp->body = NULL;
    resp->resp_formatted = NULL;

    return 0;
}

int HTTPResponse_Format(HTTPResponse_t *resp)
{
    if (resp == NULL)
        return -1;

    if (resp->body == NULL)
        return -2;

    const char *status_text;
    switch (resp->status_code)
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

    char response[16384]; // 16KB buffer for response
    resp->resp_length = snprintf(response, sizeof(response), RESPONSE_HEADER,
                                              resp->status_code, status_text, strlen(resp->body), resp->body);

                                              
    resp->resp_formatted = strdup(response);
    if (resp->resp_formatted == NULL)
        return -1;

    return 0;
}

void HTTPResponse_Dispose(HTTPResponse_t *resp)
{
    if (resp == NULL)
        return;

    if (resp->body != NULL)
    {
        free(resp->body);
        resp->body = NULL;
    }

    if (resp->resp_formatted != NULL)
    {
        free(resp->resp_formatted);
        resp->resp_formatted = NULL;
    }
}