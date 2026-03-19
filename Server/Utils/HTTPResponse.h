#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>

/**
 * @file HTTPResponse.h
 * @brief Strukturer och funktioner för HTTP-responshantering.
 * @defgroup HTTPResponse HTTP Response
 * @{
 */

/**
 * @struct HTTPResponse
 * @brief Representerar en HTTP-respons.
 * 
 * @note `response` pekar på data som ägs av anroparen.
 * @note `response_formatted` är allokerad internt och måste frigöras i Dispose.
 */
typedef struct {
    int status_code;             /**< HTTP-statuskod */
    char* response;              /**< JSON/body content (caller provides) */
    char* response_formatted;    /**< Full HTTP response (malloc'd by Format) */
    size_t response_length;      /**< Längd på den formaterade responsen */
} HTTPResponse;

/**
 * @brief Initierar en HTTPResponse-struktur.
 * @param response Pekare till HTTPResponse.
 * @return 0 vid framgång, -1 om `response` är NULL.
 */
int HTTPResponse_Initialize(HTTPResponse* response);

/**
 * @brief Formaterar en HTTPResponse till komplett HTTP-header + body.
 * @param response Pekare till HTTPResponse.
 * @return 0 vid framgång, -1 vid minnesallokeringsfel, -2 om `response->response` är NULL.
 */
int HTTPResponse_Format(HTTPResponse* response);

/**
 * @brief Frigör minne allokerat i HTTPResponse.
 * @param response Pekare till HTTPResponse.
 */
void HTTPResponse_Dispose(HTTPResponse* response);

/** @} */

#endif