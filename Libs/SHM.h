#ifndef SHM_H
#define SHM_H

#include "../Algorithm/AlgoritmProtocol.h"
#include <semaphore.h>
#include <unistd.h>

/**
 * @file SMW.h
 * @brief Shared Memory Wrapper (SMW) for managing shared memory and semaphores.
 * @defgroup SMW Shared Memory Wrapper
 * @{
 */

#define MAX 10

/**
 * @brief Initializes shared memory as writer.
 *
 * Creates and maps a shared memory region with read/write access.
 *
 * @param shared Double pointer to shared memory structure.
 * @param name Name of the shared memory object.
 * @param shm_fd File descriptor for shared memory (input/output).
 *
 * @return
 * - 0 on success
 * - -1 if shm_open fails
 * - -2 if ftruncate fails
 * - -3 if mmap fails
 *
 * @pre `shared` must be a valid pointer.
 * @pre `name` must be a valid null-terminated string.
 *
 * @post Shared memory is created and mapped.
 * @post `*shared` points to mapped region.
 *
 * @warning Overwrites existing shared memory if it already exists.
 * @note Caller is responsible for synchronization (e.g., semaphores).
 */
int SHM_InitializeWriter(AlgoritmShared **shared, const char *name, int shm_fd);

/**
 * @brief Initializes shared memory as reader.
 *
 * Opens and maps an existing shared memory region with read-only access.
 *
 * @param shared Double pointer to shared memory structure.
 * @param name Name of the shared memory object.
 * @param shm_fd File descriptor for shared memory (input/output).
 *
 * @return
 * - 0 on success
 * - -1 if shm_open fails
 * - -3 if mmap fails
 *
 * @pre Shared memory must already exist.
 *
 * @post `*shared` points to mapped read-only region.
 *
 * @warning No validation is performed on the size or structure of shared memory.
 */
int SHM_InitializeReader(AlgoritmShared **shared, const char *name, int shm_fd);

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
 *
 * @note Uses POSIX named semaphores.
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
 *
 * @pre Semaphore must already exist.
 */
int SHM_OpenSemaphore(sem_t **sem, const char *name);

/**
 * @brief Closes a semaphore.
 *
 * @param sem Double pointer to semaphore.
 *
 * @post Semaphore is closed but not unlinked.
 *
 * @note Does not set pointer to NULL.
 */
void SHM_CloseSemaphore(sem_t **sem);

/**
 * @brief Closes and unlinks a semaphore.
 *
 * @param sem Double pointer to semaphore.
 * @param name Name of the semaphore.
 *
 * @post Semaphore is closed and removed from system.
 */
void SHM_DestroySemaphore(sem_t **sem, const char *name);

/**
 * @brief Releases resources for shared memory reader.
 *
 * @param shared Double pointer to shared memory.
 * @param name Name of shared memory (unused).
 * @param shm_fd File descriptor.
 *
 * @post Memory unmapped and file descriptor closed.
 *
 * @note Does not unlink shared memory.
 */
void SHM_DisposeReader(AlgoritmShared **shared, const char *name, int shm_fd);

/**
 * @brief Releases resources for shared memory writer.
 *
 * @param shared Double pointer to shared memory.
 * @param name Name of shared memory (unused).
 * @param shm_fd File descriptor.
 *
 * @post Memory unmapped and file descriptor closed.
 *
 * @note Does not unlink shared memory.
 */
void SHM_DisposeWriter(AlgoritmShared **shared, const char *name, int shm_fd);

/** @} */

#endif