#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUF_SIZE (1024)
char **split_inline(char *line);
int shell_execute(char **args);

// PS1
#define STR(X) #X               // Stringification
#define PS1_SYNTAX STR(\\w$)
int check_PS1(char *line);
void update_prompt(char *prompt, char *line);

// PATH
char prompt[BUF_SIZE];
char path[BUF_SIZE];
int check_path(char *line);
void update_path(char *line);

// Redirection
#define NO_REDIRECT (0)
#define INPUT_REDIRECT (1)
#define OUTPUT_REDIRECT (2)
#define OUTPUT_REDIRECT_APPEND (3)
int redirect_flag = NO_REDIRECT;
char redirect_file[BUF_SIZE];


/* Feature-yukta shell 
 * Referred to: https://brennan.io/2015/01/16/write-a-shell-in-c/
 *              Class lectures, BigBlueButton Recordings
 */
int main(void) {

    char *input = NULL;     // Input line
    char **args = NULL;     // Array of arguments passed as input
    int status = 0;         // indicates whether command executed or not
    long unsigned int buf_size;
    int ps1_bit = 0;        // Bit to check if PS1 is enabled
    strcpy(path, "/usr/bin");

    do {
        // Get current directory's pwd as prompt
        if (ps1_bit == 0) {
            getcwd(prompt, BUF_SIZE);         
        }
        printf("%s$ ", prompt);

        // Redirection
        redirect_flag = NO_REDIRECT;

        // Get input from user
        if (getline(&input, &buf_size, stdin) == -1) {
            // Handling CTRL-D
            perror("Bye!");
            exit(0);
        }
        // Error handling: if user presses enter without argument
        if (strcmp(input, "\n") == 0) {
            status = 1;
            continue;
        }
        input[strlen(input) - 1] = '\0';

        // Trim line beginning for whitespaces
        int i = 0;
        while (input[i] != '\0' && (input[i] == ' ' || input[i] == '\t')) {
            i++;
        }
        input = &input[i];

        // PS1 feature
        char tmp_input[BUF_SIZE];
        strncpy(tmp_input, input, BUF_SIZE);
        switch (check_PS1(tmp_input)) {
            case 1: // \w$
                ps1_bit = 0;
                status = 1;
                continue;
            case 2: // PS1="smt else"
                ps1_bit = 1;
                status = 1;
                strncpy(tmp_input, input, BUF_SIZE);
                update_prompt(prompt, tmp_input);
                continue;
            default:
                break;
        }

        // PATH feature
        if (check_path(input) == 1) {
            update_path(input);
            status = 1;
            continue;
        }

        // Split the input as per options
        args = split_inline(input);
        
        status = shell_execute(args);

        if (!input) {
            free(input);
            input = NULL;
        }
        if (!args) {
            free(args);
            args = NULL;
        }

    } while (status);

    return 0;
}

void update_prompt(char *prompt, char *line) {
    char *token = strtok(line, "\"");
    token = strtok(NULL, "\"");
    strcpy(prompt, token);
}

// PS1="custom-prompt"
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

int check_path(char *line) {
    return (strstr(line, "PATH") == line);
}

// PATH=/usr/bin:/bin:/sbin
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

// TODO: Also check for redirection
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


int execute_shell_program(char **args) {
    int pid;
    int status;
    int buf_size = TOKEN_BUFFER_SIZE;
    // fork() syscall
    pid = vfork();

    if (pid == 0) {
        // Child process
        // First, try the command as it is
        // Redirection check
        if (redirect_flag == INPUT_REDIRECT) {
            /* Close the STDIN file, so that immediate open() call takes up
             * fd = 0 */
            close(0);
            open(redirect_file, O_RDONLY);
        } else if (redirect_flag == OUTPUT_REDIRECT) {
            /* Close the STDOUT file, so that immediate open() call takes up
             * fd = 1 */
            close(1);
            // resource: man open, man chmod
            open(redirect_file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        } else if (redirect_flag == OUTPUT_REDIRECT_APPEND) {
            close(1);
            open(redirect_file, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        }

        if (execv(args[0], args) != 0);

        /* Check each entry in PATH
         * Check if the executable file with FULL PATHNAME exists
         * If it does, run it
         * Else, check another path
         */
        char inpath[BUF_SIZE];
        // PATH=/usr/bin:/home/aneesh

        char *token = strtok(path, ":");
        strcpy(inpath, token);
        strcat(inpath, "/");
        strcat(inpath, args[0]);
        // Resource: https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c
        if (access(inpath, X_OK) == 0) {
            // Executable file exists
            // Redirection check
            if (redirect_flag == INPUT_REDIRECT) {
                /* Close the STDIN file, so that immediate open() call takes up
                 * fd = 0 */
                close(0);
                open(redirect_file, O_RDONLY);
            } else if (redirect_flag == OUTPUT_REDIRECT) {
                /* Close the STDOUT file, so that immediate open() call takes up
                 * fd = 1 */
                close(1);
                // resource: man open, man chmod
                open(redirect_file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            } else if (redirect_flag == OUTPUT_REDIRECT_APPEND) {
                close(1);
                open(redirect_file, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            }
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
                    // Executable file exists
                    // Redirection check
                    if (redirect_flag == INPUT_REDIRECT) {
                        /* Close the STDIN file, so that immediate open() call takes up
                         * fd = 0 */
                        close(0);
                        open(redirect_file, O_RDONLY);
                    } else if (redirect_flag == OUTPUT_REDIRECT) {
                        /* Close the STDOUT file, so that immediate open() call takes up
                         * fd = 1 */
                        close(1);
                        // resource: man open, man chmod
                        open(redirect_file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    } else if (redirect_flag == OUTPUT_REDIRECT_APPEND) {
                        close(1);
                        open(redirect_file, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
                    }

                    execv(inpath, args);
                    perror("execution");
                }
            }
        }
        printf("Sorry! No such executable found:(\n");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        wait(0);
    }

    return 1;
}

// In-built shell commands
// TODO: Add additional commands here
char *inbuilt_commands[] = {
    "help",
    "cd",
    "exit"
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

int execute_help(void) {
    printf("HELP: In-built programs:\n");
    for (int i = 0; i < get_total_inbuilts(); ++i) {
        printf("%s ", inbuilt_commands[i]);
    }
    putchar('\n');
    return 1;
}

int execute_cd(char **args) {
    if (args[1] == NULL) {
        if (chdir(getenv("HOME"))) {
            perror("chdir() to /home failed");
        }
    } else if (chdir(args[1])) {
        perror("chdir() failed");
    }
    return 1;
}

int execute_exit(void) {
    printf("EXIT: Bye!\n");
    return 0;
}

int shell_execute(char **args) {
    // Error check for empty command
    if (args == NULL || args[0] == NULL) {
        return 1;
    }
    // Shell in-built functions
    int i = match_inbuilts(args);
    switch(i) {
        case 0:
            return execute_help();
            break;
        case 1:
            return execute_cd(args);
        case 2:
            return execute_exit();
        default:
            break;
    }
    // Shell functions which require syscalls
    return execute_shell_program(args);
}




