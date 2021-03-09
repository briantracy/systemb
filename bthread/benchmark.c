

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "./bthread.h"


#define ARRAY_SIZE (4096 * 512)
#define BUS_SLAM_ROUNDS 1000
#define THREAD_FUNC_REPS 1000
#define NUM_THREADS 50

//#define DYNAMIC_ALLOC

#ifdef DYNAMIC_ALLOC
int *massive_array;
#else 
int massive_array[ARRAY_SIZE];
#endif

void seed() {

  #ifdef DYNAMIC_ALLOC
    massive_array = malloc(sizeof(int) * ARRAY_SIZE);
  #endif

    for (int i = 0; i < ARRAY_SIZE; i++) {
        massive_array[i] = rand() % ARRAY_SIZE;
    }
}

static unsigned long total = 0;
void slam_bus() {
    // Perform lots of writes and reads to memory
    // Try to avoid hitting caches.
    int idx = rand() % ARRAY_SIZE;
    for (int i = 0; i < BUS_SLAM_ROUNDS; i++) {
        int next_idx = massive_array[idx];
        assert(next_idx >= 0 && next_idx < ARRAY_SIZE);
        // next thread that comes here will go somewhere totally different
        massive_array[idx] = (ARRAY_SIZE - 1) - massive_array[idx];
        assert(massive_array[idx] >= 0 && massive_array[idx] < ARRAY_SIZE);
        idx = next_idx;
        total += idx;
    }
}

bthread_mutex mtx;
int protect_me = 12;

int thread_fun(void *arg) {
    (void)arg;
    for (int i = 0; i < THREAD_FUNC_REPS; i++) {
        bthread_mutex_lock(&mtx);
        assert(protect_me == 12);
        protect_me++;
        assert(protect_me == 13);
        protect_me--;
        assert(protect_me == 12);

        slam_bus();
        bthread_mutex_unlock(&mtx);
    }
    exit(0);
}

int main(int argc, char *argv[]) {

    (void)argc; (void)argv;
    seed();
    for (int i = 0; i < NUM_THREADS; i++) {
        int s = bthread_create(&thread_fun, argv[i + 1]);
        if (s == -1) {
            fprintf(stderr, "thread creation failed\n");
            exit(1);
        }
    }
    
    bthread_collect();
    fprintf(stderr, "total = %ld\n", total);
}