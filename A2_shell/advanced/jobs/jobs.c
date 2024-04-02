#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include "jobs.h"

/* Job control 
 * Resource: https://www.gnu.org/software/libc/manual/html_node/Implementing-a-Shell.html
 */
/* The active jobs are linked into a list.  This is its head */
job *first_job = NULL;

/* Find the active job with the indicated pgid */
job *find_job (pid_t pgid) {
    job *j = NULL;
    for (j = first_job; j; j = j->next)
        if (j->pgid == pgid)
            return j;
    return NULL;
}

/* Return true if all processes in the job have stopped or completed */
int job_is_stopped (job *j) {
    process *p;
    for (p = j->first_process; p; p = p->next)
        if (!p->completed && !p->stopped)
            return 0;
    return 1;
}

/* Return true if all processes in the job have completed */
int job_is_completed (job *j) {
  process *p;
  for (p = j->first_process; p; p = p->next)
    if (!p->completed)
      return 0;
  return 1;
}


/* 2 Initializing the shell */
/* Keep track of attributes of the shell.  */
pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;

/* Make sure the shell is running interactively as the foreground job
   before proceeding. */
void init_shell() {
    /* See if we are running interactively.  */
    shell_terminal = STDIN_FILENO;
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
            /* Loop until we are in the foreground.  */
            while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
                kill (- shell_pgid, SIGTTIN);

            /* Ignore interactive and job-control signals.  */
            signal(SIGINT, SIG_IGN);
            signal(SIGQUIT, SIG_IGN);
            signal(SIGTSTP, SIG_IGN);
            signal(SIGTTIN, SIG_IGN);
            signal(SIGTTOU, SIG_IGN);
            signal(SIGCHLD, SIG_IGN);

            /* Put ourselves in our own process group.  */
            shell_pgid = getpid();
            if (setpgid(shell_pgid, shell_pgid) < 0) {
                perror ("Couldn't put the shell in its own process group");
                exit (1);
            }

            /* Grab control of the terminal.  */
            tcsetpgrp(shell_terminal, shell_pgid);

            /* Save default terminal attributes for shell.  */
            tcgetattr(shell_terminal, &shell_tmodes);
    }
}


