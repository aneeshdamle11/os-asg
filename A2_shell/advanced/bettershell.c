#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define BUF_SIZE (1024)
void trim(char **line);
#define FREE(X) (free(X), X = NULL)
void execute_shell(char *line);

/* PS1 Prompt */
#define STR(X) #X               // Stringification
#define PS1_SYNTAX STR(\\w$)
int PS1_check(char *line);
int ps1_bit = 0;        // Bit to check if PS1 is enabled
char prompt[BUF_SIZE];
char *get_prompt(void);

/* PATH */
char path[BUF_SIZE];
int PATH_check(char *line);
void update_path(char *line);

/* Redirection */
#define NO_REDIRECT (0)
#define INPUT_REDIRECT (1)
#define OUTPUT_REDIRECT (2)
#define OUTPUT_REDIRECT_APPEND (3)
int redirect_flag = NO_REDIRECT;
char redirect_file[BUF_SIZE];

/* History */
char history[BUF_SIZE][BUF_SIZE];
int history_len = 0;
int history_features(char *line);

/* TODO: Job Control */
#define SUSPENDED (1)
#define RUNNING_BG (2)
void passhandler(int signo);
void inthandler(int signo);
void pausehandler(int signo);
void init_suspended_processes(void);
void execute_bg(char *line);
void execute_fg(char *line);
void store_suspended_process(char *name, int pid);

typedef struct process {
    int id;
    char name[BUF_SIZE];
    int state;
    struct process *next;
} process;

process *suspended_processes;
int current_process_id;

/* Feature-yukta shell
 * Referred to: https://brennan.io/2015/01/16/write-a-shell-in-c/
 *              Class lectures, BigBlueButton Recordings
 */
int main(void) {
    char *input = NULL;     /* Input line */

    strcpy(path, "/usr/bin");
    init_suspended_processes();

    while (1) {
        /* signal handling */
        signal(SIGINT, passhandler);
        signal(SIGTSTP, passhandler);
        /* Get current directory's pwd as prompt */
        printf("%s$ ", get_prompt());
        /* Redirection */
        redirect_flag = NO_REDIRECT;
        /* Get input from user */
        long unsigned int buf_size;
        if (getline(&input, &buf_size, stdin) == -1) {
            perror("Bye!"); /* handle Ctrl-D */
            exit(0);
        }
        /* Error handling: if user presses enter without argument */
        if (strcmp(input, "\n") == 0) {
            continue;
        }
        input[strlen(input) - 1] = '\0';
        /* Trim line beginning for whitespaces */
        trim(&input);
        /* entry in history */
        strncpy(history[history_len++], input, BUF_SIZE);
        /* handle "exit" */
        if (strcmp(input, "exit") == 0) {
            printf("EXIT: Bye!\n");
            exit(0);
        }
        /* handle "history" */
        if (history_features(input)) {
            continue;
        }
        /* PS1 feature */
        if (PS1_check(input) == 1)
            continue;
        /* PATH feature */
        if (PATH_check(input) == 1) {
            update_path(input);
            continue;
        }
        /* handle bg */
        if (strstr(input, "bg") == input) {
            execute_bg(input);
            continue;
        }
        /* handle fg */
        if (strstr(input, "fg") == input) {
            execute_fg(input);
            continue;
        }
        /* execute custom shell */
        execute_shell(input);
        /* free input string */
        FREE(input);
    };
    return 0;
}

/* trim line for beginning and ending whitespaces */
void trim(char **line) {
    char *input = *line;
    /* beginning */
    int i = 0;
    while (input[i] != '\0' && (input[i] == ' ' || input[i] == '\t')) {
        i++;
    }
    /* ending */
    int j = strlen(input) - 1;
    while (input[j] == ' ' || input[j] == '\t') {
        j--;
        if (isalnum(input[j])) {
            input[j+1] = '\0';
            break;
        }
    }
    *line = &input[i];
    return;
}

char *get_prompt(void) {
    if (ps1_bit == 0) {
        getcwd(prompt, BUF_SIZE);
    }
    return prompt;
}

void update_prompt(char *line) {
    char *token = strtok(line, "\"");
    token = strtok(NULL, "\"");
    strncpy(prompt, token, BUF_SIZE);
    return;
}

/* PS1="custom-prompt" */
int check_PS1(char *line) {
    if (!line) {
        return -1;
    }
    if (strstr(line, "PS1") == line) {
        char *token = strtok(line, "\"");
        token = strtok(NULL, "\"");
        if (!token) {
            printf("Syntax: PS1=\"custom prompt\"\n");
            return 0;
        }
        if (strcmp(token, PS1_SYNTAX) == 0) {
            return 1;
        } else {
            return 2;
        }
    } else {
        return 0;
    }
}

int PS1_check(char *line) {
    char tmp_input[BUF_SIZE];
    strncpy(tmp_input, line, BUF_SIZE);
    switch (check_PS1(tmp_input)) {
        case 1: // \w$
            ps1_bit = 0;
            return 1;
        case 2: // PS1="smt else"
            ps1_bit = 1;
            strncpy(tmp_input, line, BUF_SIZE);
            update_prompt(tmp_input);
            return 1;
        default:
            return 0;
    }
    return 0;
}

int PATH_check(char *line) {
    return (strstr(line, "PATH") == line);
}

/* PATH=/usr/bin:/bin:/sbin */
void update_path(char *line) {
    int i = 0;
    while (line[i] != '/' && line[i] != '\0') {
        i++;
    }
    if (line[i] == '\0') {
        printf("Syntax: PATH=/path_directory\nEg: PATH=/usr/bin:/sbin:/bin\n");
        return;
    }

    strcpy(path, &(line[i]));
}

#define TOKEN_BUFFER_SIZE (64)
#define TOKEN_DELIMITERS (" \t\r\n\a=")

char **split_inline(char *line) {
    int buf_size = TOKEN_BUFFER_SIZE, pos = 0;
    /* Array of strings to store command and the options that follow */
    char **tokens = (char **)malloc(sizeof(char *) * buf_size);
    if (!tokens) {
        printf("Could not malloc space for tokens\n");
        exit(EXIT_FAILURE);
    }
    /* Stores recent delimited token (to be added to tokens) */
    char *token = NULL;

    token = strtok(line, TOKEN_DELIMITERS);
    while (token != NULL) {
        /* Input redirection */
        if (strcmp(token, "<") == 0) {
            redirect_flag = INPUT_REDIRECT;
            token = strtok(NULL, TOKEN_DELIMITERS);
            strncpy(redirect_file, token, BUF_SIZE);
            break;
        }
        /* Output redirection */
        else if (strcmp(token, ">") == 0) {
            redirect_flag = OUTPUT_REDIRECT;
            token = strtok(NULL, TOKEN_DELIMITERS);
            strncpy(redirect_file, token, BUF_SIZE);
            break;
        } 
        /* Output redirection - II (Append) */
        else if (strcmp(token, ">>") == 0) {
            redirect_flag = OUTPUT_REDIRECT_APPEND;
            token = strtok(NULL, TOKEN_DELIMITERS);
            strncpy(redirect_file, token, BUF_SIZE);
            break;
        } else {
            redirect_flag = NO_REDIRECT;
        }
        tokens[pos] = token;
        pos++;
        if (pos >= buf_size) {
            buf_size += TOKEN_BUFFER_SIZE;
            tokens = realloc(tokens, buf_size * sizeof(char *));
            if (!tokens) {
                printf("Could not realloc space for tokens\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOKEN_DELIMITERS);
    }
    tokens[pos] = NULL;
    return tokens;
}

void update_redirection(int redirect_flag) {
        if (redirect_flag == INPUT_REDIRECT) {
            /* Close the STDIN file, so that immediate open() call takes up
             * fd = 0 */
            close(0);
            open(redirect_file, O_RDONLY);
        } else if (redirect_flag == OUTPUT_REDIRECT) {
            /* Close the STDOUT file, so that immediate open() call takes up
             * fd = 1 */
            close(1);
            /* resource: man open, man chmod */
            open(redirect_file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        } else if (redirect_flag == OUTPUT_REDIRECT_APPEND) {
            close(1);
            open(redirect_file, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        }
        return;
}

void execute_shell_program(char **args) {
    int buf_size = TOKEN_BUFFER_SIZE;
    /* fork() syscall
     * Child process
     * First, try the command as it is
     * Redirection check
     */
    update_redirection(redirect_flag);
    if (execv(args[0], args) != 0);

    /* Check each entry in PATH
     * Check if the executable file with FULL PATHNAME exists
     * If it does, run it
     * Else, check another path
     */
    char inpath[BUF_SIZE];

    /* PATH=/usr/bin:/home/aneesh */
    char *token = strtok(path, ":");
    strcpy(inpath, token);
    strcat(inpath, "/");
    strcat(inpath, args[0]);
    /* Resource: https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c */
    if (access(inpath, X_OK) == 0) {
        /* Executable file exists
         * Redirection check
         */
        update_redirection(redirect_flag);
        execv(inpath, args);
        perror("execution");
    } else {
        while (1) {
            token = strtok(NULL, ":");
            if (!token) {
                break;
            }
            strcpy(inpath, token);
            strcat(inpath, "/");
            strcat(inpath, args[0]);
            if (access(inpath, X_OK) == 0) {
                /* Executable file exists
                 * Redirection check
                 */
                update_redirection(redirect_flag);
                execv(inpath, args);
                perror("execution");
            }
        }
        printf("Sorry, no such command found:(\n");
        exit(0);
    }
    return;
}

/* In-built shell commands
 * TODO: Add additional commands here
 */
char *inbuilt_commands[] = {
    "help",
    "cd",
    "jobs",
    "bg"
};

int get_total_inbuilts(void) {
    return (sizeof(inbuilt_commands) / sizeof(char *));
}

int match_inbuilts(char **args) {
    int n = get_total_inbuilts();
    for (int i = 0; i < n; ++i) {
        if (strcmp(args[0], inbuilt_commands[i]) == 0) {
            return i;
        }
    }
    return -1;
}

void execute_help(void) {
    printf("HELP: In-built programs:\n");
    for (int i = 0; i < get_total_inbuilts(); ++i) {
        printf("%s ", inbuilt_commands[i]);
    }
    putchar('\n');
    exit(0);
}

void execute_cd(char **args) {
    if (args[1] == NULL) {
        if (chdir(getenv("HOME"))) {
            perror("chdir() to /home failed");
        }
    } else if (chdir(args[1])) {
        perror("chdir() failed");
    }
    exit(0);
}

void execute_jobs(char **args) {
    process *p = suspended_processes;
    int i = 0;
    while (p != NULL) {
        i++;
        if (p->state == SUSPENDED) {
            printf("[%d]\tsuspended\t%s\t%d\n", i, p->name, p->state);
        } else if (p->state == RUNNING_BG) {
            printf("[%d]\trunning\t%s\t%d\n", i, p->name, p->state);
        }
        p=p->next;
    }
    exit(0);
}

void execute_bg(char *line) {
    char **args = split_inline(line);
    if (!args || !args[0]) {
        ;
    } else if (!args[1] || strcmp(args[1], "%1") == 0) {
        process *p = suspended_processes;
        kill(p->id, SIGCONT);
        printf("[%d]  - continued %s %d\n", 1, p->name, p->state);
        p->state = RUNNING_BG;
    } else {
        char *ptr = args[1];
        if (ptr[0] == '%') {
            ptr++;
            int n = atoi(ptr);
            printf("n: %d\n", n);
            process *p = suspended_processes, *q = NULL;
            for (int i = 1; p && i < n; ++i) {
                q = p;
                p = p->next;
            }
            if (!p) {
                printf("bg: %");
                printf("%d: Sorry! No such job\n", n);
            } else {
                kill(p->id, SIGCONT);
                printf("[%d]\tcontinued\t%s\n", n, p->name);
                p->state = RUNNING_BG;
            }
        }
    }
    FREE(args);
    return;
}

void execute_fg(char *line) {
    char **args = split_inline(line);
    if (!args || !args[0]) {
        ;
    } else if (!args[1] || strcmp(args[1], "%1") == 0) {
        process *p = suspended_processes;
        suspended_processes = suspended_processes->next;
        kill(p->id, SIGCONT);
        printf("[%d]  - continued %s\n", 1, p->name);
        current_process_id = p->id;
        signal(SIGTSTP, pausehandler);
        signal(SIGINT, inthandler);
        /* waiting on children */
        int wstatus;
        waitpid(p->id, &wstatus, WUNTRACED);
        if (WIFEXITED(wstatus)) {
            ;
        } else if (WIFSTOPPED(wstatus)) {
            store_suspended_process(p->name, p->id);
        } else if (WIFCONTINUED(wstatus)) {
            /* TODO: To be handled */;
        }
        FREE(p);
    } else {
        char *ptr = args[1];
        if (ptr[0] == '%') {
            ptr++;
            int n = atoi(ptr);
            printf("n: %d\n", n);
            process *p = suspended_processes, *q = NULL;
            for (int i = 1; p != NULL && i < n; ++i) {
                q = p;
                printf("%s\n", p->name);
                p = p->next;
            }
            if (!p) {
                printf("fg: %");
                printf("%d: Sorry! No such job\n", n);
            } else {
                kill(p->id, SIGCONT);
                printf("[%d]\tcontinued\t%s\n", n, p->name);
                q->next = p->next;
                current_process_id = p->id;
                signal(SIGTSTP, pausehandler);
                signal(SIGINT, inthandler);
                /* waiting on children */
                int wstatus;
                waitpid(p->id, &wstatus, WUNTRACED);
                if (WIFEXITED(wstatus)) {
                    ;
                } else if (WIFSTOPPED(wstatus)) {
                    store_suspended_process(p->name, p->id);
                } else if (WIFCONTINUED(wstatus)) {
                    /* TODO: To be handled */
                }
                q->next = p->next;
                FREE(p);
            }
        }
    }
    FREE(args);
    return;
}

void shell_execute(char **args) {
    // Error check for empty command
    if (args == NULL || args[0] == NULL) {
        return;
    }
    // Shell in-built functions
    int i = match_inbuilts(args);
    switch(i) {
        case 0:
            execute_help();
        case 1:
            execute_cd(args);
        case 2:
            execute_jobs(args);
        default:
            break;
    }
    // Shell functions which require syscalls
    execute_shell_program(args);
    return;
}

/* === Pipes === */
/* separate piped commands and return total commands */
int extract_commands(char **cmds, char *line) {
    int n = 0, i = 0;
    char *token = NULL;

    token = strtok(line, "|");
    if (!token) {
        free(cmds);
        return n;
    }
    trim(&token);
    cmds[i++] = token;
    while (token = strtok(NULL, "|")) {
        trim(&token);
        cmds[i++] = token;
        n++;
    }
    cmds[i] = NULL;

    return n;
}

/* create n pipes and store their indexes */
int** create_pipes(int n) {
    int** pipefds = (int**) malloc(sizeof(int*) * n);
    if (!pipefds) {
        printf("Couldn't malloc pipe fds\n");
    }
    for (int i = 0; i < n; ++i) {
        pipefds[i] = (int*)malloc(sizeof(int) * 2);
    }
    for (int i = 0; i < n; ++i) {
        pipe(pipefds[i]);
    }
    return pipefds;
}

void close_all_pipes(int **pfds, int n) {
    for (int i = 0; i < n; ++i) {
        close(pfds[i][0]);
        close(pfds[i][1]);
    }
    return;
}

/* execute piped commands */
void execute_piped_commands(char **cmds, int **pipefds, int n, int i) {
    /* no process */
    if (!cmds) return;
    if (i > n) return;

    /* first command */
    if (i == 0) {
        char **args = NULL;
        int p = fork();
        if (p == 0) {
            /* execute cmd in child process */
            close(1);                   /* close the stdout */
            dup(pipefds[i][1]);         /* out == pipefds[1] or write end of pipe */
            close_all_pipes(pipefds, n);/* close all pipes */
            /* execute the command */
            args = split_inline(cmds[i]);
            shell_execute(args);
        } else {
            execute_piped_commands(cmds, pipefds, n, i+1);
        }
        if (args) {
            FREE(args);
        }
    } else if (i == n) {
        /* last command */
        char **args = NULL;
        int p = fork();
        if (p == 0) {
            /* execute cmd in child process in pipes */
            close(0);                   /* close the stdin */
            dup(pipefds[i-1][0]);       /* in == pipe read of previous pipe */
            close_all_pipes(pipefds, n);/* close all pipes */
            /* execute the command */
            args = split_inline(cmds[i]);
            shell_execute(args);
        } else {
            close_all_pipes(pipefds, n); /* close all pipes */
            /* wait for the last process to finish */
            for (int j = 0; j <= n; ++j) {
                wait(0);
            }
            if (args)
                FREE(args);
        }
    } else {
        /* all other commands */
        char **args = NULL;
        int p = fork();
        if (p == 0) {
            /* execute cmd in child process in pipes */
            close(0);                   /* close the stdin */
            dup(pipefds[i-1][0]);       /* in == pipe read of previous pipe */
            close(1);                   /* close the stdout */
            dup(pipefds[i][1]);         /* out = pipe write of current pipe */
            close_all_pipes(pipefds, n);/* close all pipes */
            /* execute the command */
            args = split_inline(cmds[i]);
            shell_execute(args);
        } else
            execute_piped_commands(cmds, pipefds, n, i+1);
        if (args)
            FREE(args);
    }
    return;
}

void store_suspended_process(char *name, int pid) {
    process *nn = (process *)malloc(sizeof(process));
    if (!nn) {
        printf("Could not malloc a process.\n");
        exit(0);
    }
    nn->id = pid;
    strncpy(nn->name, name, BUF_SIZE);
    nn->state = SUSPENDED;
    if (!suspended_processes) {
        nn->next = NULL;
        suspended_processes = nn;
    } else {
        nn->next = suspended_processes;
        suspended_processes = nn;
    }
    return;
}

void execute_single_command(char *line) {
    /* only process */
    char **args = NULL;
    // Can store the job here
    int p = fork();
    if (p == 0) {
        /* execute the command */
        args = split_inline(line);
        shell_execute(args);
    } else {
        current_process_id = p;
        signal(SIGTSTP, pausehandler);
        signal(SIGINT, inthandler);
        /* waiting on children */
        int wstatus;
        waitpid(p, &wstatus, WUNTRACED | WCONTINUED);
        if (WIFEXITED(wstatus)) {
            ;
        } else if (WIFSTOPPED(wstatus)) {
            store_suspended_process(line, p);
        } else if (WIFCONTINUED(wstatus)) {
            /* TODO: To be handled */
        }
    }
    if (args) {
        FREE(args);
    }
    return;
}

void execute_shell(char *line) {
    /* commands array to store commands */
    char **commands = (char **)malloc(sizeof(char*) * BUF_SIZE);
    if (!commands) {
        perror("cmds malloc");
        exit(EXIT_FAILURE);
    }
    int npipes = extract_commands(commands, line);
    /* Single command */
    if (npipes == 0) {
        execute_single_command(commands[0]);
    } else {
        /* pipes array to store pipes */
        int **pipefds = create_pipes(npipes);
        /* Using pipes */
        execute_piped_commands(commands, pipefds, npipes, 0);
        for (int i = 0; i < npipes; ++i) {
            if (pipefds[i]) {
                FREE(pipefds[i]);
            }
        }
        FREE(pipefds);
    }
    FREE(commands);
    return;
}

/* display history of commands */
void show_history() {
    for (int i = 0; i < history_len; ++i) {
        printf("%5d  %s\n", i, history[i]);
    }
    return;
}

int history_features(char *line) {
    char tmp[BUF_SIZE];
    strncpy(tmp, line, BUF_SIZE);
    int i;
    /* history command */
    if (strcmp(tmp, "history") == 0) {
        show_history();
        return 1;
    } else if (tmp[0] == '!') {
        /* history feature */
        int num = 0;
        for (i = 1; tmp[i] != '\0' && isdigit(tmp[i]); ++i) {
            num = num * 10 + tmp[i] - '0';
        }
        if (num >= history_len) {
            printf("Sorry! no such event: %d\n", num);
            return 1;
        }
        strncpy(line, history[num], BUF_SIZE);
        strcat(line, &(tmp[i]));
    }
    return 0;
}

/* === JOB CONTROL === */
void init_suspended_processes() {
    suspended_processes = NULL;
    return;
}

/* Ctrl-C */
void passhandler(int signo) {
    char buf[BUF_SIZE];
    strncpy(buf, prompt, BUF_SIZE);
    strcat(buf, "$ ");
    int count = strlen(buf);
    write(1, "\n", 1);
    write(1, buf, count);
    return;
}

/* Ctrl-Z */
void pausehandler(int signo) {
    kill(current_process_id, SIGSTOP);
    write(1, "\n", 1);
    return;
}

/* Ctrl-C */
void inthandler(int signo) {
    kill(current_process_id, SIGINT);
    write(1, "\n", 1);
    return;
}

