#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>  // Include this header for intptr_t

#define NUM_THREADS 3
#define MAX_TOTAL_ACCESS 15

int buffer = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int total_access = 0;

void *thread_function(void *arg) {
    int thread_id = *((int *)arg);
    int changes_count = 0;

    while (1) {
        // Acquire the mutex to ensure mutual exclusion
        pthread_mutex_lock(&mutex);

        // Check if the total access limit is reached
        if (total_access >= MAX_TOTAL_ACCESS) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Print thread ID, process ID, and buffer's current value
        printf("Thread ID %d, Process ID %d, Buffer %d\n", pthread_self(), getpid(), buffer);

        // Increment the buffer by one
        buffer++;

        // Increment the changes_count
        changes_count++;

        // Increment the total_access counter
        total_access++;

        // Release the mutex
        pthread_mutex_unlock(&mutex);

        // Wait for a short period to simulate other threads
        usleep(10000); // 10 milliseconds
    }

    // Return the number of times the buffer is changed
    return (void *)(intptr_t)changes_count;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Create three threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, (void *)&thread_ids[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; ++i) {
        void *changes_count;
        pthread_join(threads[i], &changes_count);
        printf("Thread %d changed the buffer %d times.\n", i, (int)(intptr_t)changes_count);
    }

    return 0;
}
