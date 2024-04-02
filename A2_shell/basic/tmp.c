#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	int pid;
	char *buf;
	long unsigned int buf_size;

    do {
	    printf("prompt$ ");
        if (getline(&buf, &buf_size, stdin) == -1) {
            perror("Bye!");
            exit(0);
        }
        buf[strlen(buf)-1] = '\0';
        pid = fork();
        if(pid == 0) {
            execlp(buf, buf, NULL);
        } else {
            wait(0);
        }
        fflush(stdout);
    } while (1);
    return 1;
}
