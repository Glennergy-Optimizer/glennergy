#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H


#define _POSIX_C_SOURCE 200809L

#include <stddef.h>

typedef struct{
    int status_code;
    char* response;              // JSON/body content (caller provides)
    char* response_formatted;    // Full HTTP response (malloc'd by Format)
    size_t response_length;      // Length of formatted response
}HTTPResponse;


int HTTPResponse_Initialize(HTTPResponse* response);

int HTTPResponse_Format(HTTPResponse* response);

void HTTPResponse_Dispose(HTTPResponse* response);

#endif