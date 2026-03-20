#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>

/**
 * @file HTTPResponse.h
 * @brief Structures and functions for HTTP response handling.
 * @defgroup HTTPResponse HTTP Response
 * @{
 */

/**
 * @struct HTTPResponse
 * @brief Represents an HTTP response.
 *
 * This structure is used to construct and format an HTTP response
 * including status code, body (JSON), and full HTTP string.
 *
 * @note `response` points to data owned by the caller.
 * @note `response_formatted` is allocated internally and must be freed using HTTPResponse_Dispose().
 * @note `response_length` represents the length of the formatted HTTP response (including headers).
 */
typedef struct {
    int status_code;             /**< HTTP status code */
    char* response;              /**< JSON/body content (ownership: see @note) */
    char* response_formatted;    /**< Full HTTP response (malloc'd internally) */
    size_t response_length;      /**< Length of the formatted response */
} HTTPResponse;

/**
 * @brief Initializes an HTTPResponse structure.
 *
 * Sets default values for all fields.
 *
 * @param response Pointer to HTTPResponse.
 * @return 0 on success, -1 if `response` is NULL.
 *
 * @pre `response` must be a valid pointer.
 * @post `status_code = 200`
 * @post `response = NULL`
 * @post `response_formatted = NULL`
 * @post `response_length = 0`
 *
 * @warning This function does not allocate memory.
 */
int HTTPResponse_Initialize(HTTPResponse* response);

/**
 * @brief Formats an HTTPResponse into a complete HTTP header + body.
 *
 * Constructs a full HTTP response including headers and JSON body.
 *
 * @param response Pointer to HTTPResponse.
 * @return
 * - 0 on success
 * - -1 on memory allocation failure or NULL pointer
 * - -2 if `response->response` is NULL
 *
 * @pre `response` must be initialized via HTTPResponse_Initialize().
 * @pre `response->response` must contain valid body data.
 *
 * @post `response->response_formatted` is allocated and contains full HTTP response.
 * @post `response->response_length` contains the length of the formatted response.
 *
 * @warning Allocates memory that must be freed using HTTPResponse_Dispose().
 * @note Uses an internal fixed-size buffer (2048 bytes).
 */
int HTTPResponse_Format(HTTPResponse* response);

/**
 * @brief Frees memory allocated in HTTPResponse.
 *
 * @param response Pointer to HTTPResponse.
 *
 * @pre `response` may be NULL (no effect).
 *
 * @post `response_formatted` freed and set to NULL.
 * @post `response` freed and set to NULL.
 *
 * @warning This function also frees `response`, even though it is documented
 *         as being owned by the caller. This may cause double-free issues.
 *
 * @note Safe to call multiple times (idempotent for NULL fields).
 */
void HTTPResponse_Dispose(HTTPResponse* response);

/** @} */

#endif