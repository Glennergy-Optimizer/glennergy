/**
 * @file HTTPResponse.c
 * @brief Implementation of HTTP response handling.
 * @ingroup HTTPResponse
 */

#include "HTTPResponse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Standard HTTP header-mall */
#define RESPONSE_HEADER "HTTP/1.1 %d %s\r\n"                                  \
                        "Content-Length: %zu\r\n"                             \
                        "Content-Type: application/json\r\n"                  \
                        "Access-Control-Allow-Origin: *\r\n"                  \
                        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n" \
                        "Access-Control-Allow-Headers: Content-Type\r\n"      \
                        "\r\n"                                                \
                        "%s"

/**
 * @brief Initializes an HTTPResponse structure to default values.
 * @param http_response Pointer to HTTPResponse.
 * @return 0 on success, -1 if NULL pointer.
 *
 * @pre `http_response` must be valid.
 *
 * @post status_code = 200
 * @post response = NULL
 * @post response_formatted = NULL
 * @post response_length = 0
 *
 * @note This function does not allocate memory.
 */
int HTTPResponse_Initialize(HTTPResponse *http_response)
{
    if (http_response == NULL)
        return -1;

    http_response->status_code = 200;
    http_response->response_length = 0;
    http_response->response = NULL;
    http_response->response_formatted = NULL;

    return 0;
}

/**
 * @brief Formats HTTPResponse into a complete HTTP response.
 * @param http_response Pointer to HTTPResponse.
 * @return
 * - 0 on success
 * - -1 on malloc failure or NULL pointer
 * - -2 if `response` is NULL
 *
 * @pre `http_response` must be initialized.
 * @pre `http_response->response` must be set.
 *
 * @post `http_response->response_formatted` contains full HTTP response.
 * @post `http_response->response_length` contains response length.
 *
 * @warning Internal buffer has fixed size (2048 bytes). Risk of truncation.
 * @warning Allocates memory via strdup which must be freed.
 *
 * @note Status code is mapped to text using a simple switch.
 */
int HTTPResponse_Format(HTTPResponse *http_response)
{
    if (http_response == NULL)
        return -1;

    if (http_response->response == NULL)
        return -2;

    const char *status_text;
    switch (http_response->status_code)
    {
    case 200:
        status_text = "OK";
        break;
    case 201:
        status_text = "Created";
        break;
    case 400:
        status_text = "Bad Request";
        break;
    case 404:
        status_text = "Not Found";
        break;
    case 408:
        status_text = "Connection Timeout";
        break;
    case 500:
        status_text = "Internal Server Error";
        break;
    default:
        status_text = "Unknown";
        break;
    }

    char response[2048];

    http_response->response_length = snprintf(response, sizeof(response), RESPONSE_HEADER,
                                              http_response->status_code,
                                              status_text,
                                              strlen(http_response->response),
                                              http_response->response);

    http_response->response_formatted = strdup(response);

    if (http_response->response_formatted == NULL)
        return -1;

    // Suggestion: Check if snprintf return value >= sizeof(response) to detect truncation

    return 0;
}

/**
 * @brief Frees memory allocated by HTTPResponse.
 * @param http_response Pointer to HTTPResponse.
 *
 * @pre May be NULL.
 *
 * @post response_formatted freed and set to NULL.
 * @post response freed and set to NULL.
 *
 * @warning Frees `response` even though it is documented as caller-owned.
 *          This may cause double-free issues.
 *
 * @note Safe to call multiple times.
 */
void HTTPResponse_Dispose(HTTPResponse *http_response)
{
    if (http_response == NULL)
        return;

    if (http_response->response != NULL)
    {
        free(http_response->response);
        http_response->response = NULL;
    }

    if (http_response->response_formatted != NULL)
    {
        free(http_response->response_formatted);
        http_response->response_formatted = NULL;
    }

    // Suggestion: Clarify ownership contract (either always free or never free `response`)
}