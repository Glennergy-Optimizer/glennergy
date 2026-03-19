/**
 * @file Fetcher.h
 * @brief Wrapper functions for performing HTTP GET requests using libcurl.
 * 
 * Provides APIs for initializing a CurlResponse, performing HTTP GET requests,
 * handling response data, and cleaning up resources.
 * 
 * Note: Caller is responsible for allocating CurlResponse on stack or heap.
 * 
 * @author YourName
 * @date 2026-03-19
 */

#ifndef FETCHER_H
#define FETCHER_H

#include <curl/curl.h>
#include <stddef.h>

/**
 * @brief Stores data from a Curl HTTP request.
 */
typedef struct {
    CURL *curl_handle;    /**< CURL handle */
    char *data;           /**< Pointer to response data buffer */
    size_t size;          /**< Size of response data */
} CurlResponse;

/**
 * @brief Initialize a CurlResponse structure.
 *
 * Allocates internal CURL handle and sets data pointer to NULL.
 *
 * @param _Response Pointer to CurlResponse to initialize. Must not be NULL.
 * @return 0 on success, -1 if _Response is NULL
 */
int Curl_Initialize(CurlResponse *_Response);

/**
 * @brief Callback function for writing received data into CurlResponse.
 *
 * Used internally by libcurl. Appends received data to the response buffer.
 *
 * @param contents Pointer to data received by libcurl
 * @param size Size of each element
 * @param nmemb Number of elements
 * @param userp Pointer to CurlResponse
 * @return Number of bytes processed, or 0 on allocation failure
 */
size_t Curl_WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

/**
 * @brief Perform an HTTP GET request using CurlResponse.
 *
 * @param _Response Pointer to initialized CurlResponse
 * @param url URL to request
 * @return 0 on success,
 *         -1 if _Response or curl_handle is NULL,
 *         -2 on libcurl error,
 *         -3 if HTTP response code is not 200
 */
int Curl_HTTPGet(CurlResponse *_Response, char *url);

/**
 * @brief Clean up CurlResponse resources.
 *
 * Frees response data buffer and cleans up CURL handle.
 *
 * @param _Response Pointer to CurlResponse to dispose
 */
void Curl_Dispose(CurlResponse *_Response);

#endif /* FETCHER_H */