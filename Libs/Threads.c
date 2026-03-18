#include <stdio.h>
#include <stdlib.h>
#include "Threads.h"

// Create global queue of clients
Queue global_queue;

void *Threads_Work(void *arg);

int Threads_Initialize(Threads *_ThreadPool)
{
    // Initializing queue struct, maybe move to its own function/file?
    global_queue.queue_count = 0;
    global_queue.queue_in = 0;
    global_queue.queue_out = 0;

    pthread_mutex_init(&global_queue.queue_mutex, NULL);
    pthread_cond_init(&global_queue.not_empty, NULL);
    pthread_cond_init(&global_queue.not_full, NULL);

    // Initialize all the threads, count is specified in Server.c
    for (size_t i = 0; i < POOL_SIZE; i++)
    {
        _ThreadPool[i].thread_stop = 0;
        pthread_create(&_ThreadPool[i].thread, NULL, Threads_Work, &_ThreadPool[i]);
    }

    return 0;
}

// Producer in "Producer Consumer pattern", function is used as a callback in ConnectionHandler_OnConnect()
int Threads_AddQueueItem(Connection *_Connection)
{
    printf("Add queue: \n");

    pthread_mutex_lock(&global_queue.queue_mutex);

    // Wait if the queue is full
    while (global_queue.queue_count == QUEUE_MAX)
    {
        pthread_cond_wait(&global_queue.not_full, &global_queue.queue_mutex);
    }

    // Add new clients to the queue using a circular buffer
    global_queue.client_queue[global_queue.queue_in] = _Connection;
    global_queue.queue_in = (global_queue.queue_in + 1) % QUEUE_MAX;
    global_queue.queue_count++;

    // Tell the threads there are clients to be processed
    pthread_cond_signal(&global_queue.not_empty);

    pthread_mutex_unlock(&global_queue.queue_mutex);

    return 0;
}

void *Threads_Work(void *arg)
{
    Threads *threads = (Threads *)arg;

    if (threads == NULL)
        return NULL;

    while (1)
    {
        pthread_mutex_lock(&global_queue.queue_mutex);

        // Wait for queue to get clients, check for thread_stop here if threads are stuck when shutting down
        while (global_queue.queue_count == 0 && threads->thread_stop == 0)
        {
            pthread_cond_wait(&global_queue.not_empty, &global_queue.queue_mutex);
        }

        // Unlock the mutex and break out of the loop
        if (threads->thread_stop != 0)
        {
            pthread_mutex_unlock(&global_queue.queue_mutex);
            break;
        }

        // Consumer in the "Producer Consumer pattern" grabs a client from the global queue
        Connection *client = global_queue.client_queue[global_queue.queue_out];
        global_queue.queue_out = (global_queue.queue_out + 1) % QUEUE_MAX;
        global_queue.queue_count--;

        // Signal the producer that we've taken a client and that the queue is not full
        pthread_cond_signal(&global_queue.not_full);

        pthread_mutex_unlock(&global_queue.queue_mutex);

        // Thread handles a client, disposes it and grabs another one from the queue if there is more clients

        if (client != NULL)
        {
            Connection_Handle(client);
            Connection_Dispose(&client);
        }
    }

    return NULL;
}

void Threads_Dispose(Threads *_ThreadPool)
{
    pthread_mutex_lock(&global_queue.queue_mutex);

    // Tells every thread to stop when we shutdown
    for (size_t i = 0; i < POOL_SIZE; i++)
    {
        _ThreadPool[i].thread_stop = 1;
    }

    // If threads are stuck waiting for clients, this wakes every thread so they can exit
    pthread_cond_broadcast(&global_queue.not_empty);

    pthread_mutex_unlock(&global_queue.queue_mutex);

    // Join all the threads when we are finished
    for (size_t i = 0; i < POOL_SIZE; i++)
    {
        pthread_join(_ThreadPool[i].thread, NULL);
    }
}
