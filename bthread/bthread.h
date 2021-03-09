#ifndef BTHREAD_H
#define BTHREAD_H

int bthread_create(int (*thread_func)(void *), void *thread_arg);
void bthread_collect();

typedef struct {
    volatile int locked;
} bthread_mutex;

void bthread_mutex_lock(bthread_mutex *mtx);
void bthread_mutex_unlock(bthread_mutex *mtx);


#endif
