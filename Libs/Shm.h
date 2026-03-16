// #ifndef SHM_H
// #define SHM_H


// #include "../Cache/CacheProtocol.h"
// #include <pthread.h>
// #include <time.h>



// #define SHM_NAME "/glennergy_shm"
// #define SHM_SIZE sizeof(SharedData_t)


// typedef struct SharedData_t {

//     AlgorithmResults_t results[MAX_RESULTS];
//     size_t results_count;

//     pthread_rwlock_t results_lock;

//     time_t last_update;
//     int initialized;
// } SharedData_t;

// int shm_Create();
// int shm_Destroy();

// SharedData_t* shm_Attach();
// int shm_Detach(SharedData_t *shm);

// // ===Results===
// int shm_Lock_Read(SharedData_t *shm);
// int shm_Unlock_Read(SharedData_t *shm);

// int shm_Lock_Write(SharedData_t *shm);
// int shm_Unlock_Write(SharedData_t *shm);

// AlgorithmResults_t* shm_GetResult(SharedData_t *shm, int home_id);
// int shm_UpdateResults(SharedData_t *shm, const AlgorithmResults_t *results);

// #endif