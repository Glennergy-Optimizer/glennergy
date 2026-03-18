/*#define MODULE_NAME "HTTPRequest"
#include "HTTPRequest.h"
#include "../Log/Logger.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


int HTTPRequest_Initialize(HTTPRequest* http_request)
{
    if(http_request == NULL)
        return -1;

    http_request->content_length = 0;
    http_request->path = NULL;
    http_request->request_body = NULL;
    http_request->method = HTTP_METHOD_UNKNOWN;

    return 0;
}


static int parse_content_length(const char *buffer, size_t buffer_length)
{
    const char *line = buffer;
    
    // Check each header line
    while (line < buffer + buffer_length && *line != '\r')
    {
        if (strncasecmp(line, "Content-Length:", 15) == 0)
        {
            line += 15;
            while (*line == ' ' || *line == '\t')
                line++;
            
            // Parse the number
            char *endptr;
            long length = strtol(line, &endptr, 10);
            
            if (endptr != line && length >= 0) //if digit found
                return (int)length;            //success parse
            else
                return 0;
        }

        line = strstr(line, "\r\n");
        if (line == NULL)
            break;
        line += 2;
    }
    
    return 0;
}

int HTTPRequest_ParseHeaders(HTTPRequest *http_request, const char *buffer, size_t buffer_length)
{
    if (http_request == NULL || buffer == NULL)
        return -1;

    const char *header_end = strstr(buffer, "\r\n\r\n");
    if (header_end == NULL)
        return 1; // Incomplete headers

    // Detect HTTP method
    if (strncmp(buffer, "GET ", 4) == 0)
        http_request->method = HTTP_METHOD_GET;
    else if (strncmp(buffer, "POST ", 5) == 0)
        http_request->method = HTTP_METHOD_POST;
    else if (strncmp(buffer, "OPTIONS ", 8) == 0)
        http_request->method = HTTP_METHOD_OPTIONS;
    else {
        LOG_WARNING("Unsupported HTTP method");
        return -1;
    }

    // Extract URL
    const char *path_start = strchr(buffer, ' ');
    if (path_start == NULL)
        return -1;

    path_start++;
    const char *path_end = strchr(path_start, ' ');
    if (path_end == NULL)
        return -1;

    http_request->path = strndup(path_start, path_end - path_start);
    if (http_request->path == NULL) {
        LOG_ERROR("Failed to copy path");
        return -1;
    }

    // Parse Content-Length only for methods that have a request body
    if (http_request->method == HTTP_METHOD_POST) {
        http_request->content_length = parse_content_length(buffer, buffer_length);
    }

    return 0;
}



void HTTPRequest_Dispose(HTTPRequest* http_request)
{
    if(http_request == NULL)
    return;

    if(http_request->path != NULL)
    {
        free(http_request->path);
        http_request->path = NULL;
    }

    if(http_request->request_body != NULL)
    {
        free(http_request->request_body);
        http_request->request_body = NULL;
    }
}*/