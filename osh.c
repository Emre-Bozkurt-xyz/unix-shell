#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LINE 80 /* The maximum length command */

int main(void)
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1;          /* flag to determine when to exit program */

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        /**
         * After reading user input, the steps are:
         * (1) fork a child process using fork()
         * (2) the child process will invoke execvp()
         * (3) parent will invoke wait() unless command included &
         */

        char input[MAX_LINE];
        char *line = fgets(input, MAX_LINE, stdin);
        
        // split into args w strtok, on spaces and newline
        char *token = strtok(line, " \n");
        int i = 0;
        while (token != NULL && i < MAX_LINE/2) {
            args[i++] = token;
            token = strtok(NULL, " \n");
        }
        args[i] = NULL; // Null-terminate the args array

        // Check for exit command
        if (args[0] != NULL && strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        // Fork a child process, but on trailing '&' we don't wait for it to finish
        int background = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            background = 1;
            args[i - 1] = NULL; // Remove '&' from args
        }

        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Fork failed\n");
            return 1;
        } else if (pid == 0) {
            // Child process
            if (execvp(args[0], args) == -1) {
                fprintf(stderr, "Error executing command\n");
                _exit(1);
            }
        } else {
            // Parent process
            if (!background) {
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }
    return 0;
}