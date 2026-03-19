/**
 * @file Threads.h
 * @brief Thread pool and connection queue module.
 *
 * @details
 * Implements a fixed-size thread pool using the producer-consumer pattern.
 *
 * - Incoming connections are enqueued via Threads_AddQueueItem()
 * - Worker threads dequeue and process connections
 * - Queue is bounded and implemented as a circular buffer
 *
 * Synchronization:
 * - pthread mutex protects queue access
 * - condition variables handle blocking behavior
 *
 * Blocking behavior:
 * - Producer blocks if queue is full
 * - Consumers block if queue is empty
 */

#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include "../Server/Connection/Connection.h"

/** @brief Number of worker threads in pool */
#define POOL_SIZE 20

/** @brief Maximum number of connections in queue */
#define CONNECTION_MAX 50

/** @brief Queue capacity */
#define QUEUE_MAX 50

/**
 * @brief Shared connection queue.
 *
 * @note Access must be protected by queue_mutex
 */
typedef struct {
    Connection* client_queue[QUEUE_MAX]; /**< Circular buffer of connections */
    int queue_count;                     /**< Number of items in queue */
    int queue_in;                        /**< Write index */
    int queue_out;                       /**< Read index */

    pthread_mutex_t queue_mutex;         /**< Mutex protecting queue */
    pthread_cond_t not_empty;            /**< Signaled when queue has data */
    pthread_cond_t not_full;             /**< Signaled when queue has space */
} Queue;

/**
 * @brief Worker thread structure.
 */
typedef struct {
    pthread_t thread;   /**< Thread handle */
    int thread_stop;    /**< Stop flag (non-zero = terminate) */
} Threads;

/**
 * @brief Initialize thread pool and shared queue.
 *
 * @param[out] _ThreadPool Array of thread structures
 *
 * @return 0 on success
 *
 * @pre _ThreadPool != NULL
 *
 * @post
 * - Queue is initialized
 * - Worker threads are running
 */
int Threads_Initialize(Threads* _ThreadPool);

/**
 * @brief Add connection to processing queue.
 *
 * @param[in] _Connection Connection to process
 *
 * @return 0 on success
 *
 * @pre _Connection != NULL
 *
 * @post Connection is enqueued for processing
 *
 * @note
 * - Blocks if queue is full
 * - Ownership of _Connection is transferred to the thread pool
 */
int Threads_AddQueueItem(Connection *_Connection);

/**
 * @brief Shutdown thread pool and join all threads.
 *
 * @param[in,out] _ThreadPool Thread pool
 *
 * @pre _ThreadPool != NULL
 *
 * @post All threads are stopped and joined
 *
 * @note
 * Threads will exit after completing current work or when awakened
 */
void Threads_Dispose(Threads* _ThreadPool);

#endif