#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS     101 // 100 arguments + NULL

int parse_args(char *cmd, char **args, int max_args) {
    int argc = 0;
    char *p = cmd;
    
    while (*p && argc < max_args - 1) {  // Reservar espacio para NULL
        // Saltar espacios
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        
        if (*p == '"') {
            // Argumento entre comillas
            p++; // saltar comilla inicial
            args[argc] = p;
            while (*p && *p != '"') p++;
            if (*p == '"') {
                *p = '\0';
                p++;
            }
        } else {
            // Argumento normal
            args[argc] = p;
            while (*p && *p != ' ' && *p != '\t') p++;
            if (*p) {
                *p = '\0';
                p++;
            }
        }
        argc++;
    }
    
    // Verificar si hay más argumentos después de alcanzar el límite
    while (*p == ' ' || *p == '\t') p++;
    if (*p) {
        // Hay más argumentos, excede el límite
        return -1;
    }
    
    args[argc] = NULL;
    return argc;
}

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

        int N = command_count;
        int pipes[N-1][2]; // Cree N-1 pipes en N comandos en vez de 1 solo en el padre
        for (int i = 0; i < N-1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("Error creating pipes");
                for (int j = 0; j < i; j++) { // Cerrar pipes ya creados
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                exit(EXIT_FAILURE);
            }
        }

        pid_t pids[N];
        for (int i = 0; i < N; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error executing fork");
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
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                // Reemplazar el tokenizado simple con la función que maneja comillas
                char *args[MAX_ARGS];
                int argc =parse_args(commands[i], args, MAX_ARGS);
                if (argc == -1) {
                    fprintf(stderr, "Error: too many arguments\n");
                    exit(EXIT_FAILURE);
                }
                execvp(args[0], args);
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
            
            pids[i] = pid;
        }

        // Cerrar TODOS los pipes en el padre después de crear todos los procesos
        for (int i = 0; i < N-1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        // Esperar a que terminen todos los hijos
        for (int i = 0; i < N; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }

    return 0;
}

