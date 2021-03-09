
// so we can access clone
#define _GNU_SOURCE

#include <sys/mman.h>
#include <sched.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "./bthread.h"

#define BTHREAD_STACK_SIZE (60 * 1024)
#define BTHREAD_MAX_THREADS 100

static int thread_ids[BTHREAD_MAX_THREADS];

static int next_tid = 0;

int bthread_create(int (*thread_func)(void *), void *thread_arg)
{
    if (next_tid == BTHREAD_MAX_THREADS) {
        fprintf(stderr, "you cannot create any more threads, the limit is %d\n", BTHREAD_MAX_THREADS);
        return -1;
    }

    char *const stack = mmap(
        /*
            Desired location of the memory region. If NULL, the kernel will
            choose one for you.
        */
        (void *)0,
        /*
            Minimum size of the memory region.
        */
        BTHREAD_STACK_SIZE,
        /*
            Access control on the pages that will be given back to the user.
            Note the lack of PROT_EXEC -- we are creating a stack, no need
            for it to be executable.
        */
        PROT_READ | PROT_WRITE,
        /*
            MAP_PRIVATE says that we do not want changes to our region written
            back to a file.
            MAP_ANONYMOUS says that there is no file backing this memory region
            and that it shall be initialized to 0.
        */
        MAP_PRIVATE | MAP_ANONYMOUS,
        /*
            The file descriptor we want to map in. We have no backing file,
            and MAP_ANONYMOUS requires this to be -1.
        */
        -1,
        /*
            Offset into the file we are mapping (no file == no offset).
        */
        0
    );

    if (stack == MAP_FAILED) {
        fprintf(stderr, "Could not create stack for new thread\n");
        return -1;
    }

    /*
        Stacks on Linux grow downwards (higher virtual addresses to lower ones).
        Contiguous memory regions grow upwards, so mmap returns the "bottom"
        (low virtual address) of the region it allocates.
        clone requires the top of the stack, so we compute it.
    */
    char *const stack_top = stack + BTHREAD_STACK_SIZE;

    int s = clone(
        /*
            The function that will be executed in the new thread.
        */
        thread_func,
        /*
            The top of the stack on which the new thread will begin executing.
        */
        (void *)stack_top,
        /*
            CLONE_VM forces this thread to be run in the same address space
            as the parent. This is what makes it a "thread" like thing, and not
            a separate process.
            CLONE_FS tells the new thread to use the same filesystem
	    information as the parent (cwd, root dir, ...).
            CLONE_FILES tells the new thread to share the parent thread's
            file descriptors.
            SIGCHLD is necessary so that these threads can be wait()ed upon.
        */
        CLONE_VM | CLONE_FS | CLONE_FILES | SIGCHLD,
        /*
            The argument to pass to thread_func
        */
        thread_arg
    );

    if (s == -1) {
        fprintf(stderr, "clone() failed, cannot create new thread\n");
        return -1;
    }

    thread_ids[next_tid] = s;
    next_tid++;

    return s;
}

void bthread_collect()
{
    fprintf(stderr, "collecting threads ...\n");
    if (next_tid == 0) {
        fprintf(stderr, "no threads to collect, done\n");
        exit(0);
    }
    for (int i = 0; i < next_tid; i++) {
        int status;
        if (waitpid(thread_ids[i], &status, 0) == -1) {
            fprintf(stderr, "... failed to join with thread %d\n", i);
	    continue;
        }
        fprintf(stderr, "... thread %i exited with value %d\n", i, WEXITSTATUS(status));
    }
    fprintf(stderr, "... bye\n");
}


void bthread_mutex_lock(bthread_mutex *mtx)
{
    /*
        This construct is called a "spin" lock because
        instead of putting the thread to sleep if we
        realize the mutex we are attempting to acquire is
        locked, we simply repeatedly poll it until it becomes
        unlocked.
    */
    while (1) {
        if (mtx->locked != 0) { continue; }

        // This looks good, but we need to 
        // verify with an atomic instruction
        int existing_value = -1;
        asm volatile (
            "movl $0, %%eax;"
            "movl $1, %%edx;"
            "lock cmpxchg %%edx, %0;"
            "movl %%eax, %1;"
            : "=m" (mtx->locked), "=r" (existing_value)
            :
            : "eax", "edx"
        );

        if (existing_value == 0) {
            // we got the lock
            return;
        }
    }
}
void bthread_mutex_unlock(bthread_mutex *mtx)
{
    // Only 1 thread should ever be calling this function
    // at a time, so a non-atomic write is OK.
    mtx->locked = 0;
}
