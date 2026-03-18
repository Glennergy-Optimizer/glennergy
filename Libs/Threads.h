#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include "../Server/Connection/Connection.h"

#define POOL_SIZE 20

#define CONNECTION_MAX 50

#define QUEUE_MAX 50

//Global queue struct, all threads access this
typedef struct{
    Connection* client_queue[QUEUE_MAX];
    int queue_count;
    int queue_in;
    int queue_out;
    pthread_mutex_t queue_mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
}Queue;

//Struct for each individual thread spawned
typedef struct{
    pthread_t thread;
    int thread_stop;
}Threads;

int Threads_Initialize(Threads* _ThreadPool);

//Used as callback inside ConnectionHandler_OnConnect()
int Threads_AddQueueItem(Connection *_Connection);

void Threads_Dispose(Threads* _ThreadPool);



#endif