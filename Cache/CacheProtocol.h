/**
 * @file CacheProtocol.h
 * @brief IPC protocol definitions for cache communication.
 *
 * @details
 * Defines request/response structures used over UNIX socket
 * between clients and the cache service.
 *
 * @defgroup CACHEPROTOCOL Cache Protocol
 * @ingroup IPC
 * @{
 *
 * @note All structures are packed and ABI-sensitive.
 * @note Communication occurs via UNIX socket at CACHE_SOCKET_PATH.
 * @warning Clients and server must use identical struct layout.
 */

#ifndef CACHE_PROTOCOL_H
#define CACHE_PROTOCOL_H

#include <stdint.h>
#include "../Libs/GlennergyPaths.h"

/**
 * @brief UNIX socket path used by cache service.
 *
 * @note Must be consistent between clients and server.
 */
#define CACHE_SOCKET_PATH GLENNERGY_CACHE_SOCKET_PATH

/**
 * @brief Supported cache commands.
 *
 * @note Values are fixed and must not change to maintain compatibility.
 */
typedef enum {
    CMD_GET_ALL = 1,       /**< Request all cached data */
    CMD_GET_METEO = 2,     /**< Request only Meteo data */
    CMD_GET_SPOTPRIS = 3,  /**< Request only Spotpris data */
    CMD_PING = 99          /**< Health check / ping */
} CacheCommand;

/**
 * @brief Cache request message structure.
 *
 * Used by clients to request data from the cache service.
 *
 * @note All fields must be sent in network byte order if cross-machine.
 * @note `reserved` is for future expansion and must be zeroed.
 *
 * @pre `command` must be one of CacheCommand values.
 * @post Server interprets message according to `command`.
 */
typedef struct {
    uint8_t command;     /**< Command type */
    uint8_t reserved[3]; /**< Padding / future use, must be 0 */
} CacheRequest;

/**
 * @brief Cache response header.
 *
 * Sent by the cache service to the client before the payload.
 *
 * @note `status` is 0 for success, non-zero for errors.
 * @note `data_size` indicates number of bytes following the header.
 *
 * @pre Server must fill `status` and `data_size` before sending.
 * @post Client uses `data_size` to read the payload.
 */
typedef struct {
    uint32_t status;     /**< 0 = success, 1 = error */
    uint32_t data_size;  /**< Payload size in bytes */
} CacheResponse;

/** @} */

#endif
