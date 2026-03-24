/**
 * @file AlgoritmProtocol.h
 * @brief Shared memory structures and protocol definitions for Algorithm module.
 * @defgroup Algorithm Algorithm Module
 * @ingroup Algorithm
 *
 * This file defines the shared memory layout and communication protocol for
 * the Algorithm module.
 * 
 * @note Original comments preserved.
 */

#ifndef TESTREADER_H
#define TESTREADER_H

#define MAX_ID 5                        /**< Maximum number of results in shared memory */
#define ALGORITM_SHARED "/algoritm_shm" /**< Shared memory name */
#define ALGORITM_MUTEX "/algoritm_mutex" /**< Semaphore name for shared memory */

/**
 * @brief Placeholder function for testing reader
 *
 * @return 0 on success, -1 on error
 */
int test_reader();

/**
 * @brief Time representation for a sample
 * @note Memory owned by parent structure; array size 32.
 */
typedef struct
{
    char time[32]; /**< Timestamp string (YYYY-MM-DD HH:MM) */
} time_start;

/**
 * @brief Result per ID in Algorithm module
 * @note `recommendation` array stores values for 96 quarters
 */
typedef struct
{
    int id;
    double recommendation[96];
    time_start time[96];
} AlgoritmResult;

/**
 * @brief Shared memory structure for Algorithm module
 * @note Memory ownership managed by writer/reader; array size MAX_ID
 */
typedef struct
{
    AlgoritmResult result[MAX_ID]; /**< Array of results */
} AlgoritmShared;

#endif