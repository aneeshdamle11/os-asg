#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "thread.h"

/* thread_create: starts a new thread in the calling process */
int thread_create(int *thread, int (*start_routine)(void *), void *arg) {
    void *stack = malloc(STACK_SIZE);
    if (!stack) {
        printf("Unable to malloc stack area\n");
        return -1;
    }
    *thread = clone(start_routine, stack + STACK_SIZE, 0, arg);
    if (*thread == -1) {
        printf("Unable to clone new thread\n");
        return -1;
    }
    printf("New thread created. ID = %d\n", *thread);
    return 0;
}

/* thread_join: waits for the specified thread to terminate */
int thread_join(int thread) {
    int retval = waitpid(thread, NULL, __WCLONE);
    if (retval == -1)
        return -1;
    printf("Child joined after waiting. ID = %d\n", retval);
    return 0;
}

/* thread_exit: terminates the callign thread */
void thread_exit(void *retval) {
    printf("caller's thread ID: %d\n", gettid());
    printf("Killing the thread\n");
    tgkill(gettid(), gettid(), SIGINT);
}

