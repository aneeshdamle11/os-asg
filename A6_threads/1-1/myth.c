#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "myth.h"
myth_t myth_create(myth_t *thread, int (*fn) (void *), void *arg) {
	int tid;
	char *stack = malloc(4096);
	tid = clone(fn, stack + 4096, 0, arg);
	*thread = tid;
	printf("thread id = %d\n", *thread);
	return tid;
}
int myth_join(myth_t thread) {
	printf("waitpid ret = %d\n", waitpid(thread, NULL, __WCLONE));
}
