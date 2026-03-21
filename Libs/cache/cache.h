/**
 * @file cache.h
 * @brief Filesystem-based JSON cache with TTL support.
 *
 * @details
 * Provides persistent cache entries stored as JSON files on disk.
 * Each entry is stored as:
 *   <cache_dir>/<key>.json
 *
 * Features:
 * - TTL (Time-To-Live) validation
 * - Atomic writes (via temporary file + rename)
 * - Optional thread-safety via pthread mutex
 *
 * No internal memory caching is performed.
 *
 * @ingroup CACHE
 */
#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

/**
 * @brief Cache instance structure.
 */
typedef struct Cache {
    char *cache_dir;          /**< Base directory for cache files */
    time_t ttl;               /**< Time-To-Live in seconds (0 = no expiry) */
    pthread_mutex_t mutex;    /**< Mutex for thread-safety */
} Cache;

/**
 * @brief Initialize cache instance.
 *
 * @param[out] cache Cache instance
 * @param[in] cache_dir Directory path for cache storage
 * @param[in] set_ttl TTL in seconds
 *
 * @return 0 on success, -1 on failure
 *
 * @pre cache != NULL
 * @pre cache_dir != NULL
 * @post Cache is initialized and ready for use
 * @note Directory must exist prior to initialization
 */
int cache_Init(Cache *cache, const char *cache_dir, time_t set_ttl);

/**
 * @brief Check if a cache entry exists and is still valid.
 *
 * @param[in] cache Cache instance
 * @param[in] key Cache key
 *
 * @return true if entry exists and not expired, false otherwise
 *
 * @pre cache != NULL
 * @pre key != NULL
 * @note TTL enforced if ttl > 0
 */
bool cache_IsValid(Cache *cache, const char *key);

/**
 * @brief Retrieve cached JSON data.
 *
 * @param[in] cache Cache instance
 * @param[in] key Cache key
 * @param[out] data Pointer to heap-allocated buffer (caller must free)
 * @param[out] size Size of returned data in bytes
 *
 * @return true if data was successfully retrieved, false otherwise
 *
 * @pre cache != NULL
 * @pre key != NULL
 * @pre data != NULL
 * @pre size != NULL
 * @post *data contains the file content
 * @note Returns false if file does not exist or is expired
 */
bool cache_Get(Cache *cache, const char *key, char **data, size_t *size);

/**
 * @brief Store JSON data in cache.
 *
 * @param[in] cache Cache instance
 * @param[in] key Cache key
 * @param[in] data Data buffer
 * @param[in] size Size of data in bytes
 *
 * @return 0 on success, -1 on failure
 *
 * @pre cache != NULL
 * @pre key != NULL
 * @pre data != NULL
 * @post Data written atomically (temp file + rename)
 */
int cache_Set(Cache *cache, const char *key, const char *data, size_t size);

/**
 * @brief Cleanup cache resources.
 *
 * @param[in,out] cache Cache instance
 *
 * @post Mutex destroyed, cache_dir freed
 */
void cache_Dispose(Cache *cache);

#endif // CACHE_H