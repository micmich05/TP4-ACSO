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

//necesito pipe que comunique proceso n con proceso n+1 (total n-1 pipes)
//necesito un array de pipes para almacenar los pipes que comunican los procesos
//proceso_n = fork()
// if proceso_n ==0:
//    dup2(pipe_hacia_n+1, STDOUT_FILENO)
//    cierro pipes que no se usan   
//    preparo args del proceso actual (n) para llamar a excevp
//    execvp(commands[i], args)
//    si sigue aca es porque execvp fallo

// proceso_n+1 = fork()
// if proceso_n+1 == 0:
//    dup2(pipe_hacia_n, STDIN_FILENO)
//    cierro pipes que no se usan
//    preparo args del proceso actual (n+1) para llamar a excevp
//    execvp(commands[i+1], args)

//El proceso padre debe esperar a que terminen todos los hijos y cerrar los pipes que no se usan

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS     50

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count;

    while (1) {

        printf("Shell> ");
        fflush(stdout);
        if (!fgets(command, sizeof(command), stdin))
            break;  
        command[strcspn(command, "\n")] = '\0';

        command_count = 0;
        char *tok = strtok(command, "|");
        while (tok && command_count < MAX_COMMANDS) {
            // trim espacios inicio/fin
            while (*tok == ' ') tok++;
            char *end = tok + strlen(tok) - 1;
            while (end > tok && *end == ' ') *end-- = '\0';

            commands[command_count++] = tok;
            tok = strtok(NULL, "|");
        }

        if (command_count == 0) continue;

        // Si tengo N comandos, necesito N-1 pipes
        int N = command_count;
        int pipes[N-1][2];
        for (int i = 0; i < N-1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        // Necesito un fork y execvp por cada comando
        pid_t pids[N];
        for (int i = 0; i < N; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                // --- en el hijo: redirijo stdin/stdout según posición ---
                if (i > 0) {
                    dup2(pipes[i-1][0], STDIN_FILENO);
                }
                if (i < N-1) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
                // cierro todas las copias de los pipes en el hijo
                for (int j = 0; j < N-1; j++) {
                    close(pipes[j][0]);  // aqui
                    close(pipes[j][1]);  // aqui
                }

                // Tokenizar este comando en args[] para ejecutar execvp
                char *args[MAX_ARGS];
                int argc = 0;
                char *t2 = strtok(commands[i], " ");
                while (t2 && argc < MAX_ARGS-1) {
                    args[argc++] = t2;
                    t2 = strtok(NULL, " ");
                }
                args[argc] = NULL;

                execvp(args[0], args);
                // Si sigue acá es porque execvp falló
                perror("execvp");
                exit(EXIT_FAILURE);
            }

            // --- en el padre: cierro al vuelo los extremos de pipe que ya no necesito ---
            if (i > 0) {
                close(pipes[i-1][0]);  // aqui
            }
            if (i < N-1) {
                close(pipes[i][1]);    // aqui
            }

            pids[i] = pid;
        }

        // Ya no quedan descriptores de pipe abiertos en el padre

        // Esperar a que terminen todos los hijos
        for (int i = 0; i < N; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }

    return 0;
}

