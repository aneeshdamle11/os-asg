#ifndef JOBS_H
#define JOBS_H (1)

/* 1 Data structures for the Shell */
/* A process is a single process */
typedef struct process {
    struct process *next;       /* next process in pipeline */
    char *args;                 /* for exec */
    pid_t pid;                  /* process ID */
    char completed;             /* true if process has completed */
    char stopped;               /* true if process has stopped */
    int status;                 /* reported status value */
}   process;

/* A job is a pipeline of processes */
typedef struct job {
    struct job *next;           /* next active job */
    char *command;              /* command line, used for messages */
    process *first_process;     /* list of processes in this job */
    pid_t pgid;                 /* process group ID */
    char notified;              /* true if user told about stopped job */
    struct termios tmodes;      /* saved terminal modes */
    int stdin, stdout, stderr;  /* standard i/o channels */
} job;

/*! Utility functions to operate on job objects */

/* Find the active job with the indicated pgid */
job *find_job (pid_t pgid);
/* Return true if all processes in the job have stopped or completed */
int job_is_stopped (job *j);
/* Return true if all processes in the job have completed */
int job_is_completed (job *j);

/* 2 Initializing the shell */
void init_shell(void);

#endif
