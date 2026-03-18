#include "Fetcher.h"
#include <string.h>
#include <stdlib.h>

// Förr hade vi att Init skapar en malloc CurlResponse och returnerar pekaren.
// Kan inte detta ligga på stacken istället?
// Nu har vi att Init tar en pekare till CurlResponse och initierar den.
// Det betyder att anroparen måste skapa CurlResponse på stacke eller heapen själv, sedan kalla denna för att initiera.
int Curl_Initialize(CurlResponse *_Response)
{
    if (!_Response)
    {
        return -1;
    }

    _Response->data = NULL;
    _Response->size = 0;
    _Response->curl_handle = curl_easy_init();
    

    return 0;
}

size_t Curl_WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t real_size = size * nmemb;
    CurlResponse *response = (CurlResponse *)userp;

    char *ptr = (char *)realloc(response->data, response->size + real_size + 1);
    if (ptr == NULL)
    {
        return 0;
    }

    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, real_size);
    response->size += real_size;
    response->data[response->size] = '\0';

    return real_size;
}

int Curl_HTTPGet(CurlResponse *_Response, char *url)
{
    if (!_Response || !_Response->curl_handle)
        return -1;

    curl_easy_reset(_Response->curl_handle);

    curl_easy_setopt(_Response->curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(_Response->curl_handle, CURLOPT_WRITEFUNCTION, Curl_WriteCallback);
    curl_easy_setopt(_Response->curl_handle, CURLOPT_WRITEDATA, _Response);

    CURLcode res = curl_easy_perform(_Response->curl_handle);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Curl_HTTPGet failed: %s\n", curl_easy_strerror(res));
        return -2;
    }

    long http_code = 0;
    curl_easy_getinfo(_Response->curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200)
    {
        fprintf(stderr, "HTTP request failed with code %ld\n", http_code);
        return -3;
    }

    return 0;
}

void Curl_Dispose(CurlResponse *_Response)
{
    if (!_Response)
        return;

    if (_Response->data != NULL)
        free(_Response->data);

    if (_Response->curl_handle != NULL)
        curl_easy_cleanup(_Response->curl_handle);
    // free(_Response->data);
    _Response->data = NULL;
    _Response->size = 0;
}