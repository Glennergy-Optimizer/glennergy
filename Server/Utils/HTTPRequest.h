// #ifndef HTTP_REQUEST_H
// #define HTTP_REQUEST_H

// #define _POSIX_C_SOURCE 200809L

// #include <stddef.h>

// typedef enum {
//     HTTP_METHOD_UNKNOWN = 0,
//     HTTP_METHOD_GET,
//     HTTP_METHOD_POST,
//     HTTP_METHOD_OPTIONS
// } HTTPMethod;

// typedef struct
// {
//     HTTPMethod method;
//     char *path;
//     int content_length;
//     char *request_body;
// } HTTPRequest_t;


// int HTTPRequest_Initialize(HTTPRequest_t* http_request);

// int HTTPRequest_ParseHeaders(HTTPRequest_t *http_request, const char *buffer, size_t buffer_length);

// void HTTPRequest_Dispose(HTTPRequest_t* http_request);


// #endif
