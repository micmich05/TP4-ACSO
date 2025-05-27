// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <string.h>

// #define MAX_COMMANDS 200

// int main() {

//     char command[256];
//     char *commands[MAX_COMMANDS];
//     int command_count = 0;

//     while (1) 
//     {
//         printf("Shell> ");
        
//         /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
//         fgets(command, sizeof(command), stdin);
        
//         /* Removes the newline character (\n) from the end of the string stored in command, if present. 
//            This is done by replacing the newline character with the null character ('\0').
//            The strcspn() function returns the length of the initial segment of command that consists of 
//            characters not in the string specified in the second argument ("\n" in this case). */
//         command[strcspn(command, "\n")] = '\0';

//         /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
//            Each resulting token is stored in the commands[] array. 
//            The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
//            In each iteration of the while loop, strtok() returns the next token found in command. 
//            The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
//         char *token = strtok(command, "|");
//         while (token != NULL) 
//         {
//             commands[command_count++] = token;
//             token = strtok(NULL, "|");
//         }

//         /* You should start programming from here... */
//         for (int i = 0; i < command_count; i++) 
//         {
//             printf("Command %d: %s\n", i, commands[i]);
//         }    
//     }
//     return 0;
// }
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count;

    while (1) {
        printf("Shell> ");
        if (!fgets(command, sizeof(command), stdin)) {
            // EOF or read error
            break;
        }

        // Remove trailing newline
        command[strcspn(command, "\n")] = '\0';

        // Tokenize the command string using '|' as delimiter
        command_count = 0;
        char *token = strtok(command, "|");
        while (token != NULL && command_count < MAX_COMMANDS) {
            // Trim leading spaces
            while (*token == ' ') token++;
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }
        if (command_count == 0) {
            continue;  // empty line
        }

        // Create pipes: one pipe between each pair of commands
        int pipefds[MAX_COMMANDS-1][2];
        for (int i = 0; i < command_count - 1; i++) {
            if (pipe(pipefds[i]) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        // Fork a process for each command
        pid_t pids[MAX_COMMANDS];
        for (int i = 0; i < command_count; i++) {
            pids[i] = fork();
            if (pids[i] < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pids[i] == 0) {
                // --- Child process ---

                // If not the first command, redirect stdin to previous pipe's read end
                if (i > 0) {
                    if (dup2(pipefds[i-1][0], STDIN_FILENO) < 0) {
                        perror("dup2 stdin");
                        exit(EXIT_FAILURE);
                    }
                }
                // If not the last command, redirect stdout to current pipe's write end
                if (i < command_count - 1) {
                    if (dup2(pipefds[i][1], STDOUT_FILENO) < 0) {
                        perror("dup2 stdout");
                        exit(EXIT_FAILURE);
                    }
                }

                // Close all pipe fds in the child
                for (int j = 0; j < command_count - 1; j++) {
                    close(pipefds[j][0]);
                    close(pipefds[j][1]);
                }

                // Split commands[i] into args[]
                char *args[256];
                int arg_count = 0;
                char *arg = strtok(commands[i], " ");
                while (arg != NULL) {
                    args[arg_count++] = arg;
                    arg = strtok(NULL, " ");
                }
                args[arg_count] = NULL;

                // Execute the command
                execvp(args[0], args);
                // If execvp returns, an error occurred
                perror("execvp");
                exit(EXIT_FAILURE);
            }
            // Parent continues to next command
        }

        // --- Parent process ---
        // Close all pipe fds
        for (int i = 0; i < command_count - 1; i++) {
            close(pipefds[i][0]);
            close(pipefds[i][1]);
        }

        // Wait for all children to finish
        for (int i = 0; i < command_count; i++) {
            int status;
            waitpid(pids[i], &status, 0);
        }

        // Reset for next loop iteration
        command_count = 0;
    }

    return 0;
}
