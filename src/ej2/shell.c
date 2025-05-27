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
            // Ctrl+D o error de lectura → salimos
            printf("\n");
            break;
        }

        // Eliminar '\n' final
        command[strcspn(command, "\n")] = '\0';

        // ----------------------------
        // PARTIDA: parsing por pipes
        // ----------------------------
        command_count = 0;
        char *token = strtok(command, "|");
        while (token != NULL && command_count < MAX_COMMANDS) {
            // quitar espacios iniciales
            while (*token == ' ') token++;
            // quitar espacios finales
            char *end = token + strlen(token) - 1;
            while (end > token && *end == ' ') { *end = '\0'; end--; }

            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        if (command_count == 0)
            continue;  // nada que ejecutar

        int num_cmds = command_count;
        int pipefds[2 * (num_cmds - 1)];

        // 1) Crear los pipes necesarios: uno entre cada par de comandos
        for (int i = 0; i < num_cmds - 1; i++) {
            if (pipe(pipefds + i*2) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        // 2) Fork + redirecciones para cada comando
        for (int i = 0; i < num_cmds; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                // ——— En el hijo ———
                // Si no es el primer comando: stdin viene del pipe anterior
                if (i > 0) {
                    if (dup2(pipefds[(i-1)*2], STDIN_FILENO) < 0) {
                        perror("dup2 stdin");
                        exit(EXIT_FAILURE);
                    }
                }
                // Si no es el último comando: stdout va al siguiente pipe
                if (i < num_cmds - 1) {
                    if (dup2(pipefds[i*2 + 1], STDOUT_FILENO) < 0) {
                        perror("dup2 stdout");
                        exit(EXIT_FAILURE);
                    }
                }

                // Cerrar todos los descriptores de pipe en el hijo
                for (int j = 0; j < 2*(num_cmds - 1); j++)
                    close(pipefds[j]);

                // Parsear el comando en args para execvp
                char *args[MAX_COMMANDS];
                int argc = 0;
                char *arg = strtok(commands[i], " ");
                while (arg) {
                    args[argc++] = arg;
                    arg = strtok(NULL, " ");
                }
                args[argc] = NULL;

                // Ejecutar
                execvp(args[0], args);
                // Si execvp retorna, hubo error
                perror("execvp");
                exit(EXIT_FAILURE);
            }
            // ——— En el padre: seguimos creando procesos
        }

        // 3) En el padre, cerrar todos los extremos de pipe
        for (int i = 0; i < 2*(num_cmds - 1); i++)
            close(pipefds[i]);

        // 4) Esperar a todos los hijos
        for (int i = 0; i < num_cmds; i++)
            wait(NULL);

        // Repetir para la próxima línea de comando
    }

    return 0;
}
