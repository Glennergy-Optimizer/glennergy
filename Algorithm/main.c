#define MODULE_NAME "ALGORITHM"
#include "../Server/Log/Logger.h"
#include "../Server/SignalHandler.h"
#include "testreader.h"
#include <stdio.h>
#include <stdlib.h>
#include "../Libs/SHM.h"

int main()
{
    log_Init("algorithm.log");
    SignalHandler_Initialize();
    LOG_INFO("Starting Algorithm module...");

    SharedMemory *shm;
    int shm_fd = -1;
    sem_t *mutex;

    if (SHM_InitializeWriter(&shm, ALGORITM_SHARED, shm_fd) != 0)
    {
        printf("Error algo!\n");
        return -1;
    }

    if (SHM_CreateSemaphore(&mutex, ALGORITM_MUTEX) != 0)
    {
        printf("Error algo 2!\n");
        return -2;
    }

    int ret = test_reader(shm, mutex);

    LOG_INFO("Algorithm module shutting down...");
    printf("Free cache\n");
    SHM_CloseSemaphore(&mutex);
    SHM_DisposeWriter(&shm, ALGORITM_SHARED, shm_fd);
    log_Cleanup();
    return ret;
}