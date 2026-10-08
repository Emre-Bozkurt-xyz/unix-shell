#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LINE 80 /* The maximum length command */
#define HIST_SIZE 10 /* Number of commands kept in history */

int main(void)
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1;          /* flag to determine when to exit program */

    char history[HIST_SIZE][MAX_LINE]; // Ring buffer of full command lines
    int history_count = 0;             // Total commands ever entered

    while (should_run) {
        while (waitpid(-1, NULL, WNOHANG) > 0); // Reap any finished background processes

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

        // Check for EOF (Ctrl+D)
        if (line == NULL) {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0'; // Strip newline

        // Oldest command number still held in the ring buffer
        int oldest = history_count > HIST_SIZE ? history_count - HIST_SIZE + 1 : 1;

        // !! reruns the last command, !N reruns command number N
        if (input[0] == '!') {
            if (history_count == 0) {
                printf("No commands in history.\n");
                continue;
            }

            int n = (strcmp(input, "!!") == 0) ? history_count : atoi(input + 1);
            if (n < oldest || n > history_count) {
                printf("No such command in history.\n");
                continue;
            }

            strcpy(input, history[(n - 1) % HIST_SIZE]);
            printf("%s\n", input); // Echo the command being rerun
        }

        char raw[MAX_LINE];
        strcpy(raw, input); // Copy before strtok mangles input

        // split into args w strtok, on spaces
        char *token = strtok(input, " ");
        int i = 0;
        while (token != NULL && i < MAX_LINE/2) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // Null-terminate the args array

        // On trailing '&' we don't wait for the child to finish
        int background = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            background = 1;
            args[i - 1] = NULL; // Remove '&' from args
        }

        // If no command is entered, continue to next iteration
        if (args[0] == NULL) {
            continue;
        }

        // Record every non-empty command, overwriting the oldest
        strcpy(history[history_count % HIST_SIZE], raw);
        history_count++;

        // Built-ins run in the parent, no fork
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        if (strcmp(args[0], "history") == 0) {
            oldest = history_count > HIST_SIZE ? history_count - HIST_SIZE + 1 : 1;
            for (int n = history_count; n >= oldest; n--) { // Newest first
                printf("%d %s\n", n, history[(n - 1) % HIST_SIZE]);
            }
            continue;
        }

        fflush(stdout); // Don't let the child inherit unflushed output
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Fork failed\n");
            return 1;
        } else if (pid == 0) {
            // Child process
            execvp(args[0], args);
            perror(args[0]);
            _exit(1);
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
