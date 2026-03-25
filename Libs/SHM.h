#ifndef SHM_H
#define SHM_H

#include "../Algorithm/AlgoritmProtocol.h"
#include <semaphore.h>
#include <unistd.h>

#define MAX 10

int SHM_InitializeWriter(AlgoritmShared **shared, const char *name, int shm_fd);


int SHM_InitializeReader(AlgoritmShared **shared, const char *name, int shm_fd);


int SHM_CreateSemaphore(sem_t **sem, const char *name);


int SHM_OpenSemaphore(sem_t **sem, const char *name);

// Closes semaphore
void SHM_CloseSemaphore(sem_t **sem);

// Closes and unlinks semaphore
void SHM_DestroySemaphore(sem_t **sem, const char *name);


void SHM_DisposeReader(AlgoritmShared **shared, const char *name, int shm_fd);


void SHM_DisposeWriter(AlgoritmShared **shared, const char *name, int shm_fd);


#endif