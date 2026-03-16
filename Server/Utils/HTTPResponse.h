// #ifndef HTTP_RESPONSE_H
// #define HTTP_RESPONSE_H


// #define _POSIX_C_SOURCE 200809L

// #include <stddef.h>

// typedef struct{
//     int status_code;
//     char* resp_formatted;    // Full HTTP response (malloc'd by Format)
//     char* body;              // JSON/body content (caller provides)
//     size_t resp_length;      // Length of formatted response
// }HTTPResponse_t;


// int HTTPResponse_Initialize(HTTPResponse_t* resp);

// int HTTPResponse_Format(HTTPResponse_t* resp);

// void HTTPResponse_Dispose(HTTPResponse_t* resp);

// #endif