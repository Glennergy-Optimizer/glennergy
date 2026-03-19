/**
 * @file Fetcher.c
 * @brief Implementation of HTTP GET wrapper functions using libcurl.
 *
 * Provides initialization, write callback, HTTP GET execution, and cleanup
 * for CurlResponse structures. Caller must allocate CurlResponse.
 * 
 * @author YourName
 * @date 2026-03-19
 */

#include "Fetcher.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Initialize a CurlResponse structure.
 *
 * @param _Response Pointer to CurlResponse to initialize
 * @return 0 on success, -1 if _Response is NULL
 */
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

/**
 * @brief Callback for writing received data into CurlResponse.
 *
 * Appends incoming data to the existing buffer and reallocates as needed.
 *
 * @param contents Pointer to data received
 * @param size Size of one element
 * @param nmemb Number of elements
 * @param userp Pointer to CurlResponse
 * @return Number of bytes written, 0 on allocation failure
 */
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

/**
 * @brief Perform an HTTP GET request.
 *
 * @param _Response Pointer to initialized CurlResponse
 * @param url URL string to fetch
 * @return 0 on success,
 *         -1 if _Response or curl_handle is NULL,
 *         -2 on libcurl error,
 *         -3 if HTTP status is not 200
 */
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

/**
 * @brief Clean up a CurlResponse.
 *
 * Frees memory allocated for data and cleans up CURL handle.
 *
 * @param _Response Pointer to CurlResponse to dispose
 */
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