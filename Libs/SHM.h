#ifndef SHM_H
#define SHM_H

#include "../Algorithm/AlgoritmProtocol.h"
#include <semaphore.h>
#include <unistd.h>

/**
 * @file SHM.h
 * @brief Shared memory and semaphore helpers.
 *
 * @defgroup SMW Shared Memory Wrapper
 * @brief Shared memory and semaphore helpers.
 * @{
 */

#define MAX 10

/**
 * @brief Initializes shared memory for writing.
 *
 * Creates and maps a shared memory region with read/write access.
 *
 * @param shared Double pointer to shared memory structure.
 * @param name Name of the shared memory object.
 *
 * @return
 * - 0 on success
 * - -1 if shm_open fails
 * - -2 if ftruncate fails
 * - -3 if mmap fails
 *
 * @post `*shared` points to the mapped region on success.
 * @warning Overwrites existing shared memory if it already exists.
 */
int SHM_InitializeWriter(AlgoritmShared **shared, const char *name);

/**
 * @brief Initializes shared memory for reading.
 *
 * Opens and maps an existing shared memory region with read-only access.
 *
 * @param shared Double pointer to shared memory structure.
 * @param name Name of the shared memory object.
 *
 * @return
 * - 0 on success
 * - -1 if shm_open fails
 * - -3 if mmap fails
 *
 * @post `*shared` points to the mapped read-only region on success.
 * @warning The mapped size is assumed to match `AlgoritmShared`.
 */
int SHM_InitializeReader(AlgoritmShared **shared, const char *name);

/**
 * @brief Creates a named semaphore.
 *
 * @param sem Double pointer to semaphore.
 * @param name Name of the semaphore.
 *
 * @return
 * - 0 on success
 * - -1 on failure
 *
 * @post Semaphore is created with initial value 1.
 */
int SHM_CreateSemaphore(sem_t **sem, const char *name);

/**
 * @brief Opens an existing named semaphore.
 *
 * @param sem Double pointer to semaphore.
 * @param name Name of the semaphore.
 *
 * @return
 * - 0 on success
 * - -1 on failure
 */
int SHM_OpenSemaphore(sem_t **sem, const char *name);

/**
 * @brief Closes a semaphore.
 *
 * @param sem Double pointer to semaphore.
 */
void SHM_CloseSemaphore(sem_t **sem);

/**
 * @brief Closes and unlinks a semaphore.
 *
 * @param sem Double pointer to semaphore.
 * @param name Name of the semaphore.
 */
void SHM_DestroySemaphore(sem_t **sem, const char *name);

/**
 * @brief Releases resources for shared memory reader.
 *
 * @param shared Double pointer to shared memory.
 */
void SHM_DisposeReader(AlgoritmShared **shared);

/**
 * @brief Releases resources for shared memory writer.
 *
 * @param shared Double pointer to shared memory.
 */
void SHM_DisposeWriter(AlgoritmShared **shared);

/**
 * @brief Removes shared memory object from the system.
 *
 * @param name Name of shared memory object.
 */
void SHM_Destroy(const char *name);

/** @} */

#endif

// Suggestion: Consider adding return codes and error logging for SHM_Destroy.
