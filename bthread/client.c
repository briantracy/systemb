

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "./bthread.h"

#define ITERS 800000

void print(char *greeting, char *name) {
    char buf[100];
    snprintf(&buf[0], 100, "%s from pid %d, name=%s\n", greeting, getpid(),
             name);
    if (write(1, buf, strlen(buf)) < 0) {
      perror("write");
    }
}



bthread_mutex mtx;
// This variable will be modified by all threads
int protect_me = 12;
// Total number of times a thread enters the critical
// section. Used to make sure all the threads actually
// get a chance to modify the shared memory.
int critical_section_count = 0;

int thread_fun(void *arg) {
    print("hello", (char *)arg);
    for (int i = 0; i < ITERS; i++) {
        bthread_mutex_lock(&mtx);
        critical_section_count++;
        assert(protect_me == 12);
        protect_me++;
        assert(protect_me == 13);
        protect_me--;
        assert(protect_me == 12);
        bthread_mutex_unlock(&mtx);
    }
    print("goodbye", (char *)arg);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s [threadname1] ... [threadnameN]\n", argv[0]);
        return 1;
    }
    const int nthreads = argc - 1;

    printf("I am the parent, my pid is %d\n", getpid());

    for (int i = 0; i < nthreads; i++) {
        int s = bthread_create(&thread_fun, argv[i + 1]);
        if (s == -1) {
            fprintf(stderr, "thread creation failed\n");
            bthread_collect();
            exit(1);
        }
    }
    
    bthread_collect();
    assert(critical_section_count == nthreads * ITERS);
}
