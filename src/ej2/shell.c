#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS     101 // 100 arguments + NULL

int parse_args(char *cmd, char **args, int max_args) {
    """
    Recibe un comando y prepara los argumentos para execvp.
    El comando puede contener argumentos entre comillas.
    Devuelve -1 si hay demasiados argumentos.
    """
    
    int argc = 0;
    char *p = cmd;
    
    while (*p && argc < max_args - 1) {  
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        
        if (*p == '"') {
            //caso en que el argumento está entre comillas
            p++; //saltar comilla inicial
            args[argc] = p;
            while (*p && *p != '"') p++;
            if (*p == '"') {
                *p = '\0';
                p++;
            }
        } else {
            //caso en que el argumento no está entre comillas
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
            while (*tok == ' ') tok++;
            char *end = tok + strlen(tok) - 1;
            while (end > tok && *end == ' ') *end-- = '\0';

            commands[command_count++] = tok;
            tok = strtok(NULL, "|");
        }

        if (command_count == 0) continue;

        int N = command_count;
        int pipes[N-1][2]; //creo los pipes necesarios (N-1 pipes para N comandos)
        for (int i = 0; i < N-1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("Error creating pipes");
                for (int j = 0; j < i; j++) { //cierro pipes ya fueron creados
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                exit(EXIT_FAILURE);
            }
        }

        pid_t pids[N];
        for (int i = 0; i < N; i++) {
            pid_t pid = fork(); //creo un hijo para cada comando
            if (pid < 0) {
                perror("Error executing fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                //redirijo la entrada/salida según corresponda
                if (i > 0) { //si no es el primer comando
                    dup2(pipes[i-1][0], STDIN_FILENO);
                }
                if (i < N-1) { //si no es el último comando
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
                //cierro todas las copias de los pipes
                for (int j = 0; j < N-1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                //preparo los argumentos para execvp (la funcion parse_args maneja el caso de comillas) y 
                //devuelve -1 si hay demasiados argumentos

                char *args[MAX_ARGS];
                int argc = parse_args(commands[i], args, MAX_ARGS);
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

        //cierro los pipes en el padre
        for (int i = 0; i < N-1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        //espero a que terminen todos los hijos
        for (int i = 0; i < N; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }

    return 0;
}

