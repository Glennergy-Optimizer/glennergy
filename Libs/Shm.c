#define _XOPEN_SOURCE 700
#define MODULE_NAME "SHM"
#include "../Server/Log/Logger.h"
#include "Shm.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int shm_Create() 
{
    LOG_INFO("processing shared memory...");

    // int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    // if (shm_fd >= 0) {
    //     LOG_INFO("Connected to existing shared memory");
    //     close(shm_fd);
    //     return 0;
    // }
    // if (errno != ENOENT) {
    //     LOG_WARNING("shm exists but couldn't open (errno=%d), removing stale shm", errno);
    //     shm_unlink(SHM_NAME);  // Remove stale/corrupted shm
    // }
    LOG_INFO("creating new shared memory...");
    
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_EXCL, 0666);
    if (shm_fd < 0) {
        LOG_ERROR("shm_open() failed: %s", strerror(errno));
        return -1;
    }
    
    LOG_DEBUG("Shm setting size to %zu bytes", sizeof(SharedData_t));
    if (ftruncate(shm_fd, sizeof(SharedData_t)) < 0) {
        LOG_ERROR("ftruncate() failed: %s", strerror(errno));
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return -1;
    }
    LOG_DEBUG("Mapping shared memory...");
    SharedData_t *shm_ptr = mmap(NULL, sizeof(SharedData_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    if (shm_ptr == MAP_FAILED) {
        LOG_ERROR("mmap() failed: %s", strerror(errno));
        shm_unlink(SHM_NAME);
        return -1;
    }

    memset(shm_ptr, 0, sizeof(SharedData_t));
    LOG_DEBUG("Initializing rwlocks...");

    pthread_rwlockattr_t attr;
    pthread_rwlockattr_init(&attr);
    pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    //pthread_rwlock_init(&shm_ptr->input_lock, &attr);
    pthread_rwlock_init(&shm_ptr->results_lock, &attr);

    pthread_rwlockattr_destroy(&attr);

    shm_ptr->initialized = 1;
    shm_ptr->last_update = time(NULL);
    shm_ptr->results_count = 0;

    LOG_DEBUG("Unmapping shared memory from creator process...");
    munmap(shm_ptr, sizeof(SharedData_t));

    LOG_INFO("Shared memory created (%zu bytes, perm: 0666)", sizeof(SharedData_t));
    return 0;
}

int shm_Destroy()
{
    LOG_INFO("Destroying shared memory...");
    if (shm_unlink(SHM_NAME) < 0) {
        if (errno != ENOENT) {
            LOG_ERROR("shm_unlink() failed: %s", strerror(errno));
            return -1;
        }
        LOG_WARNING("shm_unlink() Shared memory already removed");
    }
    
    LOG_INFO("Shared memory destroyed");
    return 0;
}

SharedData_t* shm_Attach() 
{
    LOG_INFO("Attaching to shared memory...");

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd < 0) {
        LOG_ERROR("shm_open() failed: %s", strerror(errno));
        return NULL;
    }
    LOG_DEBUG("Mapping shared memory...");
    SharedData_t *shm_ptr = mmap(NULL, sizeof(SharedData_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    if (shm_ptr == MAP_FAILED) {
        LOG_ERROR("mmap() failed: %s", strerror(errno));
        return NULL;
    }

    if (!shm_ptr->initialized) {
        LOG_ERROR("Shared memory not initialized");
        munmap(shm_ptr, sizeof(SharedData_t));
        return NULL;
    }
    LOG_INFO("Attached to shared memory");
    return shm_ptr;
}

int shm_Detach(SharedData_t *shm)
{
    LOG_INFO("Detaching from shared memory");
    if (!shm) {
        LOG_WARNING("shm_Detach() called with NULL pointer");
        return -1;
    }
    if (munmap(shm, sizeof(SharedData_t)) < 0) {
        LOG_ERROR("munmap() failed: %s", strerror(errno));
        return -1;
    }
    LOG_INFO("Detached from shared memory");
    return 0;
}

// ===Results read===
int shm_Lock_Read(SharedData_t *shm)
{   
    if (!shm) {
        LOG_ERROR("shm_Lock_Read() called with NULL pointer");
        return -1;
    }
    LOG_INFO("Locking results for read...");
    return pthread_rwlock_rdlock(&shm->results_lock);
}
int shm_Unlock_Read(SharedData_t *shm)
{   
    if (!shm) {
        LOG_ERROR("shm_Unlock_Read() called with NULL pointer");
        return -1;
    }
    LOG_INFO("Unlocking results read lock...");
    return pthread_rwlock_unlock(&shm->results_lock);
}
// ===Results write===
int shm_Lock_Write(SharedData_t *shm)
{   
    if (!shm) {
        LOG_ERROR("shm_Lock_Write() called with NULL pointer");
        return -1;
    }
    LOG_INFO("Locking results for write...");
    return pthread_rwlock_wrlock(&shm->results_lock);
}
int shm_Unlock_Write(SharedData_t *shm)
{   
    if (!shm) {
        LOG_ERROR("shm_Unlock_Write() called with NULL pointer");
        return -1;
    }
    LOG_INFO("Unlocking results write lock...");
    return pthread_rwlock_unlock(&shm->results_lock);
}   

// ===Helper to get results for a specific home_id===
AlgorithmResults_t* shm_GetResult(SharedData_t *shm, int home_id)
{
    LOG_INFO("Getting results for home_id %d", home_id);
    if (!shm)
        return NULL;

    for (size_t i = 0; i < shm->results_count; i++) {
        if (shm->results[i].home_id == home_id) {
            LOG_INFO("Found results for home_id %d", home_id);
            return &shm->results[i];
        }
    }
    LOG_WARNING("No results found for home_id %d", home_id);
    return NULL;
}

int shm_UpdateResults(SharedData_t *shm, const AlgorithmResults_t *result)
{
    if (!shm || !result) {
        LOG_ERROR("Invalid parameters for shm_UpdateResults");
        return -1;
    }

    // Find existing entry or add new one
    size_t idx = shm->results_count;
    for (size_t i = 0; i < shm->results_count; i++) {
        if (shm->results[i].home_id == result->home_id) {
            idx = i;
            break;
        }
    }

    if (idx > MAX_RESULTS) {
        LOG_ERROR("Maximum homes exceeded (%d)", MAX_RESULTS);
        return -1;
    }

    // Update result
    memcpy(&shm->results[idx], result, sizeof(AlgorithmResults_t));
    
    if (idx == shm->results_count) {
        shm->results_count++;
        LOG_INFO("Added new result for home_id=%d (count=%zu)", 
                 result->home_id, shm->results_count);
    } else {
        LOG_DEBUG("Updated result for home_id=%d", result->home_id);
    }

    shm->last_update = time(NULL);
    return 0;
}