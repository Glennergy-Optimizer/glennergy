/**
 * @file HTTPRequest.h
 * @brief Public API for the HTTPRequest module.
 *
 * Provides structures and functions to perform non-blocking HTTP header reads,
 * parse headers, and manage HTTP request memory.
 */

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>

/**
 * @defgroup HTTPREQUEST HTTPRequest
 * @brief Handling of HTTP requests and headers.
 * @{
 */

/**
 * @brief Enum for the result of reading from a connection.
 */
typedef enum{
  Connection_ReadResult_Success, /**< Read completed successfully. */
  Connection_ReadResult_Pending, /**< Read incomplete; more data expected. */
  Connection_ReadResult_Error,   /**< Read failed due to error. */
} Connection_ReadResult;

/**
 * @brief Structure representing an HTTP request.
 *
 * @note This struct owns its memory for url and request_body.
 */
typedef struct
{
  char *url; /**< Parsed URL from request line. */

  char recv_buffer[2048]; /**< Buffer for incoming data. */
  size_t recv_buffer_length; /**< Current length of valid data in buffer. */

  int content_length; /**< Content-Length header value. */

  char *request_body; /**< Optional HTTP request body. */
} HTTPRequest;

/**
 * @brief Initializes an HTTPRequest structure.
 *
 * @param http_request Pointer to structure to initialize.
 * @return 0 on success, -1 if http_request is NULL.
 *
 * @pre http_request must not be NULL.
 * @post All fields are zeroed or set to NULL.
 */
int HTTPRequest_Initialize(HTTPRequest* http_request);

/**
 * @brief Reads HTTP headers from a socket into the request buffer.
 *
 * @param socket File descriptor of the socket.
 * @param http_request Pointer to HTTPRequest struct.
 * @param bytesReadOut Output parameter for number of bytes read.
 * @return Connection_ReadResult indicating success, pending, or error.
 *
 * @pre http_request must be initialized.
 * @post recv_buffer contains newly read data.
 * @warning Non-blocking read; may return pending if no data available.
 */
int HTTPRequest_ReadHeaders(int socket, HTTPRequest *http_request, int* bytesReadOut);

/**
 * @brief Parses HTTP headers from the buffer.
 *
 * Extracts the request URL and validates the method and line endings.
 *
 * @param http_request Pointer to HTTPRequest struct.
 * @return 0 on success, -1 on failure, 1 if headers incomplete.
 *
 * @pre http_request must contain valid recv_buffer data.
 * @post http_request->url is allocated and populated on success.
 * @warning Caller must free url using HTTPRequest_Dispose.
 */
int HTTPRequest_ParseHeader(HTTPRequest* http_request);

/**
 * @brief Frees resources associated with an HTTPRequest.
 *
 * @param http_request Pointer to HTTPRequest struct.
 *
 * @post url and request_body pointers are freed and set to NULL.
 */
void HTTPRequest_Dispose(HTTPRequest* http_request);

/** @} */

#endif