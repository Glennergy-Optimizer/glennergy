#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>

typedef enum{
  Connection_ReadResult_Success,
  Connection_ReadResult_Pending,
  Connection_ReadResult_Error,

}Connection_ReadResult;

typedef struct
{
  char *url;

  char recv_buffer[2048];
  size_t recv_buffer_length;

  int content_length;

  
  char *request_body; 
} HTTPRequest;


int HTTPRequest_Initialize(HTTPRequest* http_request);

int HTTPRequest_ReadHeaders(int socket, HTTPRequest *http_request, int* bytesReadOut);

int HTTPRequest_ParseHeader(HTTPRequest* http_request);

void HTTPRequest_Dispose(HTTPRequest* http_request);


#endif