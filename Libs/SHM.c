#define MODULE_NAME "SHM"
#include "SHM.h"
#include "../Server/Log/Logger.h"
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


int SHM_InitializeWriter(AlgoritmShared **shared, const char *name, int shm_fd)
{
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

    if (shm_fd < 0)
    {
        LOG_ERROR("Failed to create shm");
        return -1;
    }

    if (ftruncate(shm_fd, sizeof(AlgoritmShared)) < 0)
    {
        LOG_ERROR("ftruncate failed");
        return -2;
    }

    *shared = mmap(NULL, sizeof(AlgoritmShared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (*shared == MAP_FAILED)
    {
        LOG_ERROR("Failed to create mapping");
        return -3;
    }

    LOG_INFO("Successfully created shared memory region");
    return 0;
}

int SHM_InitializeReader(AlgoritmShared **shared, const char *name, int shm_fd)
{
    shm_fd = shm_open(name, O_RDONLY, 0);

    if (shm_fd < 0)
    {
        LOG_ERROR("Failed to create shm");
        return -1;
    }


    *shared = mmap(NULL, sizeof(AlgoritmShared), PROT_READ, MAP_SHARED, shm_fd, 0);
    if (*shared == MAP_FAILED)
    {
        LOG_ERROR("Failed to create mapping");
        return -3;
    }

    LOG_INFO("Successfully opened shared memory region");
    return 0;
}

int SHM_CreateSemaphore(sem_t **sem, const char *name)
{
    *sem = sem_open(name, O_CREAT, 0666, 1);

    if (*sem == SEM_FAILED)
    {
        LOG_ERROR("Failed to create semaphore");
        return -1;
    }
    LOG_INFO("Created semaphore");
    return 0;
}

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

// Closes semaphore
void SHM_CloseSemaphore(sem_t **sem)
{
    sem_close(*sem);
    LOG_INFO("Closed semaphore");
}

// Closes and unlinks semaphore
void SHM_DestroySemaphore(sem_t **sem, const char *name)
{
    sem_close(*sem);
    sem_unlink(name);
}

void SHM_DisposeReader(AlgoritmShared **shared, const char *name, int shm_fd)
{
    (void)name; // Unused parameter, can be used for logging if needed
    munmap(*shared, sizeof(AlgoritmShared));
    close(shm_fd);
}

void SHM_DisposeWriter(AlgoritmShared **shared, const char *name, int shm_fd)
{
    (void)name; // Unused parameter, can be used for logging if needed
    munmap(*shared, sizeof(AlgoritmShared));
    close(shm_fd);
}

void SHM_Destroy(const char *name)
{
    shm_unlink(name);
}