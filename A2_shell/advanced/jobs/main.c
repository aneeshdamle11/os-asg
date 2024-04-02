#include <stdio.h>
#include <stdlib.h>
#include "jobs.h"

int main() {
    char *line = NULL;
    long unsigned int buf_size;
    init_shell();

    while (1) {
        printf("prompt$ ");
        /* get input from user */
        if (getline(&line, &buf_size, stdin) == -1) {
            perror("Bye!"); /* Ctrl^D */
            exit(0);
        }
        line[strlen(line) - 1] = '\0';
        printf("%d\n", strlen(line));

        if (strcmp(line, "exit") == 0) {
            printf("EXIT: Bye!\n");
            exit(EXIT_SUCCESS);
        }

        /* make a job */

        /* fork-exec to execute child process */
        int p = fork();
        if (p == 0) {
            /* TODO */
            int pid, pgid, foreground = 1;
            if (shell_is_interactive) {
                pid = getpid ();
                pgid = shell_pgid;
                if (pgid == 0) pgid = pid;
                setpgid(pid, pgid);
                if (foreground)
                  tcsetpgrp(shell_terminal, pgid);

                /* Set the handling for job control signals back to the default.  */
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
                signal(SIGCHLD, SIG_DFL);
            }
            execlp(line, line, NULL);
            perror("execvp");
            exit(1);
        } else {
            wait(0);
        }
    }
}
