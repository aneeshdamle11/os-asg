#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>

/* external array to store cmd-args */
#define BUF_SIZE (1024)



// Redirection
#define NO_REDIRECT (0)
#define INPUT_REDIRECT (1)
#define OUTPUT_REDIRECT (2)
#define OUTPUT_REDIRECT_APPEND (3)
int redirect_flag = NO_REDIRECT;
char redirect_file[BUF_SIZE];

#define TOKEN_BUFFER_SIZE (64)
#define TOKEN_DELIMITERS (" \t\r\n\a=")



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

void pipe_cmds(char **cmds);
int **create_pipes(int npipes);
void execute_pipe_cmds(char **cmds, int **pipefds, int n, int i);
void execute_pipe_cmds_iterative(char **cmds, int **pipefds, int n);


int main() {
    char *line = NULL;
    long unsigned int buf_size;
    char **cmds;

    printf("prompt$ ");
    while (1) {
        cmds = (char **)malloc(sizeof(char*) * BUF_SIZE);
        if (!cmds) {
            perror("cmds malloc");
            exit(EXIT_FAILURE);
        }

        /* get input */
        if (getline(&line, &buf_size, stdin) == -1) {
            perror("CTRL-D");
            exit(0);
        }
        line[strlen(line) - 1] = '\0';

        int pos = 0, npipes = 0;

        /* Parse the string and store each separate command */
        char *token = NULL;
        token = strtok(line, "|");
        if (!token)
            return 1;
        trim(&token);
        cmds[pos++] = token;
        while (token = strtok(NULL, "|")) {
            trim(&token);
            cmds[pos++] = token;
            npipes++;
        }
        cmds[pos] = NULL;

        int **pipefds = create_pipes(npipes);

        /* Using pipes */
        execute_pipe_cmds(cmds, pipefds, npipes, 0);
        wait(0);

	    /* free everything */
        free(line);
        line = NULL;
        free(token);
        token = NULL;
        free(cmds);
        cmds = NULL;
        for (int i = 0; i < npipes; ++i) {
            free(pipefds[i]);
            pipefds[i] = NULL;
        }
        free(pipefds);
        pipefds = NULL;

        printf("prompt$ ");
    }

    return 0;
}

/* Testing codes */
void test_pipefds(int **pipefds, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d-%d\n", pipefds[i][0], pipefds[i][1]);
    }
}

void test_cmds(char **cmds) {
     for (int i = 0; cmds[i]; ++i) {
        printf("cmd%d:%s\n", i, cmds[i]);
    }
}


char **split_inline(char *line) {
    int buf_size = TOKEN_BUFFER_SIZE, pos = 0;
    // Array of strings to store command and the options that follow
    char **tokens = (char **)malloc(sizeof(char *) * buf_size);
    if (!tokens) {
        printf("Could not malloc space for tokens\n");
        exit(EXIT_FAILURE);
    }
    // Stores recent delimited token (to be added to tokens)
    char *token = NULL;

    token = strtok(line, TOKEN_DELIMITERS);
    while (token != NULL) {
        // TODO: Check for errors
        // Input redirection
        if (strcmp(token, "<") == 0) {
            redirect_flag = INPUT_REDIRECT;
            token = strtok(NULL, TOKEN_DELIMITERS);
            strncpy(redirect_file, token, BUF_SIZE);
            break;
        }
        // Output redirection
        else if (strcmp(token, ">") == 0) {
            redirect_flag = OUTPUT_REDIRECT;
            token = strtok(NULL, TOKEN_DELIMITERS);
            strncpy(redirect_file, token, BUF_SIZE);
            break;
        } 
        // Output redirection - II (Append)
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

void execute_pipe_cmds(char **cmds, int **pipefds, int n, int i) {
    /* no process */
    if (!cmds) return;
    if (i > n) return;
    /* only process */
    if (n == 0) {
        char **args = NULL;
        int p = fork();
        if (p == 0) {
            /* execute the command */
            args = split_inline(cmds[0]);
            execvp(args[0], args);
        } else {
            wait(0);
        }
        if (args) {
            free(args);
            args = NULL;
        }
        return;
    }

    /* first command */
    if (i == 0) {
        char **args = NULL;
        int p = fork();
        if (p == 0) {
            /* execute cmd in child process */
            close(1);                   /* close the stdout */
            dup(pipefds[i][1]);         /* out == pipefds[1] or write end of pipe */
            close_all_pipes(pipefds, n); /* close all pipes */
            /* execute the command */
            args = split_inline(cmds[i]);
            execvp(args[0], args);
        } else {
            execute_pipe_cmds(cmds, pipefds, n, i+1);
        }
        if (args) {
            free(args);
            args = NULL;
        }
    } else if (i == n) {
        /* last command */
        char **args = NULL;
        int p = fork();
        if (p == 0) {
            /* execute cmd in child process in pipes */
            close(0);               /* close the stdin */
            dup(pipefds[i-1][0]);   /* in == pipe read of previous pipe */
            close_all_pipes(pipefds, n); /* close all pipes */
            /* execute the command */
            args = split_inline(cmds[i]);
            execvp(args[0], args);
        } else {
            close_all_pipes(pipefds, n); /* close all pipes */
            /* wait for the last process to finish */
            for (int j = 0; j < n; ++j) {
                wait(0);
            }
            if (args) {
                free(args);
                args = NULL;
            }
        }
    } else {
        /* all other commands */
        char **args = NULL;
        int p = fork();
        if (p == 0) {
            /* execute cmd in child process in pipes */
            close(0);               /* close the stdin */
            dup(pipefds[i-1][0]);   /* in == pipe read of previous pipe */
            close(1);               /* close the stdout */
            dup(pipefds[i][1]);     /* out = pipe write of current pipe */
            close_all_pipes(pipefds, n); /* close all pipes */
            /* execute the command */
            args = split_inline(cmds[i]);
            execvp(args[0], args);
        } else {
            execute_pipe_cmds(cmds, pipefds, n, i+1);
        }
        if (args) {
            free(args);
            args = NULL;
        }
    }
 
    return;
}


