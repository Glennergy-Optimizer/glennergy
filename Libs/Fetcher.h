#ifndef FETCHER_H
#define FETCHER_H

#include <stddef.h>
#include <curl/curl.h>

typedef struct{
    char *data;
    size_t size;
    CURL* curl_handle; // Lägg till en CURL-handle för att kunna återanvända den om det behövs
}CurlResponse;

int Curl_Initialize(CurlResponse *_Response);

int Curl_HTTPGet(CurlResponse *_Response, char* url);

void Curl_Dispose(CurlResponse* _Response);

#endif // CURL_H