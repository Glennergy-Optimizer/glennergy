/**
 * @file SHM.c
 * @brief Implementation of shared memory and semaphore utilities.
 *
 * @ingroup SMW
 */

#define MODULE_NAME "SHM"
#include "SHM.h"
#include "../Server/Log/Logger.h"
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

/**
 * @brief Implementation of SHM_InitializeWriter.
 *
 * See header for full contract documentation.
 */
int SHM_InitializeWriter(AlgoritmShared **shared, const char *name)
{
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0600);

    if (shm_fd < 0)
    {
        LOG_ERROR("Failed to create shm");
        return -1;
    }

    if (ftruncate(shm_fd, sizeof(AlgoritmShared)) < 0)
    {
        close(shm_fd);
        LOG_ERROR("ftruncate failed");
        return -2;
    }

    *shared = mmap(NULL, sizeof(AlgoritmShared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (*shared == MAP_FAILED)
    {
        LOG_ERROR("Failed to create mapping");
        close(shm_fd);
        return -3;
    }
    close(shm_fd);

    // Start shared memory in known empty state after we mapped so we don't have old values that survive restarts to our Reader
    memset(*shared, 0, sizeof(AlgoritmShared));

    LOG_INFO("Successfully created shared memory region");

    return 0;
}

/**
 * @brief Implementation of SHM_InitializeReader.
 *
 * See header for full contract documentation.
 */
//int SHM_InitializeReader(AlgoritmShared **shared, const char *name, int shm_fd)
int SHM_InitializeReader(AlgoritmShared **shared, const char *name)
{
    int shm_fd = shm_open(name, O_RDONLY, 0);

    if (shm_fd < 0)
    {
        LOG_ERROR("Failed to create shm");
        return -1;
    }

    *shared = mmap(NULL, sizeof(AlgoritmShared), PROT_READ, MAP_SHARED, shm_fd, 0);
    if (*shared == MAP_FAILED)
    {
        LOG_ERROR("Failed to create mapping");
        close(shm_fd);
        return -3;
    }
    close(shm_fd);

    LOG_INFO("Successfully opened shared memory region");

    return 0;

    // Suggestion: Validate that shared memory size matches expected struct size
}

/**
 * @brief Creates a semaphore.
 */
int SHM_CreateSemaphore(sem_t **sem, const char *name)
{
    *sem = sem_open(name, O_CREAT, 0600, 1);

    if (*sem == SEM_FAILED)
    {
        LOG_ERROR("Failed to create semaphore");
        return -1;
    }

    LOG_INFO("Created semaphore");

    return 0;
}

/**
 * @brief Opens an existing semaphore.
 */
int SHM_OpenSemaphore(sem_t **sem, const char *name)
{
    *sem = sem_open(name, 0);

    if (*sem == SEM_FAILED)
    {
        LOG_ERROR("Failed to open semaphore");
        return -1;
    }

    LOG_INFO("Opened semaphore");

    return 0;
}

/**
 * @brief Closes a semaphore.
 *
 * @param sem Double pointer to semaphore.
 *
 * @post Semaphore closed.
 */
void SHM_CloseSemaphore(sem_t **sem)
{
    sem_close(*sem);
    LOG_INFO("Closed semaphore");

    // Suggestion: Set *sem = NULL after closing to avoid dangling pointer
}

/**
 * @brief Closes and unlinks a semaphore.
 *
 * @param sem Double pointer to semaphore.
 * @param name Name of semaphore.
 *
 * @post Semaphore closed and unlinked.
 */
void SHM_DestroySemaphore(sem_t **sem, const char *name)
{
    sem_close(*sem);
    sem_unlink(name);

    // Suggestion: Set *sem = NULL after destruction
}

/**
 * @brief Disposes shared memory reader resources.
 */
void SHM_DisposeReader(AlgoritmShared **shared)
{
    //(void)name; // Unused parameter, can be used for logging if needed
    
    munmap(*shared, sizeof(AlgoritmShared));
    //close(shm_fd);
    //close(shm_fd);

    // Suggestion: Set *shared = NULL after munmap
}

/**
 * @brief Disposes shared memory writer resources.
 */
void SHM_DisposeWriter(AlgoritmShared **shared)
{
    //(void)name; // Unused parameter, can be used for logging if needed

    munmap(*shared, sizeof(AlgoritmShared));
    //close(shm_fd);

    // Suggestion: Set *shared = NULL after munmap
}

/**
 * @brief Removes shared memory object from the system.
 *
 * @param name Name of shared memory object.
 *
 * @post Shared memory unlinked.
 * @warning Should only be called by owner process.
 */
void SHM_Destroy(const char *name)
{
    shm_unlink(name);

    // Suggestion: Add logging for successful/failed unlink
}
