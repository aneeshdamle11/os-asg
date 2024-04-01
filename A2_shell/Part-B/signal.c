#include <stdio.h>
#include <signal.h>

int *p = 1234, i = 3456;

void seghandler(int signo) {
    printf("Segfault occurred.\n");
    return;
}

void inthandler(int signo) {
    printf("INT signal received.\n");
    return;
}

int main(void) {
    signal(SIGINT, inthandler);
    getchar(); // press ctrl-C during this
    i = 10;
    signal(SIGSEGV, seghandler);
    *p = 100;
    return 0;
}

