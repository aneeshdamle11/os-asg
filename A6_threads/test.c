#include <stdio.h>
#include <stdlib.h>
#include "thread.h"

int fn(void *x) {
    int p = *(int *)x;
    printf("Thread executing from here: %d\n", p);
    printf("Hello there! Inside fn(): %d\n", p);
    thread_exit(NULL);
    printf("Prints after exit(): %d\n", p);
    return 0;
}

int main(void) {
    int tid[3];
    for (int i = 0; i < 3; ++i) {
        if (thread_create(tid + i, fn, &i) == -1) {
            return 1;
        }
    }
    for (int i = 0; i < 3; ++i) {
        if (thread_join(tid[i]) == -1) {
            return 2;
        }
    }
    return 0;
}


