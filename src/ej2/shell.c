#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 101 // 100 arguments + NULL

int parse_args(char *cmd, char **args) {
    int argc = 0;
    char *p = cmd;
    
    while (*p && argc < MAX_ARGS - 1) {  
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        
        if (*p == '"' || *p == '\'') {  //manejar ambas comillas
            char quote = *p;  
            p++; //saltar comilla inicial
            args[argc] = p;
            
            //contenido dentro de comillas SIN procesar escapes
            while (*p && *p != quote) {
                p++;
            }
            
            if (*p == quote) {
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
    
    if (argc >= MAX_ARGS) {
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
        
        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        fgets(command, sizeof(command), stdin);

        /* Removes the newline character (\n) from the end of the string stored in command, if present. */
        command[strcspn(command, "\n")] = '\0';
        
        //adaptado para manejar el regex y espacios al principio y final
        command_count = 0;
        char *start = command;
        int i = 0;
        int in_quotes = 0;
        char quote_char = '\0';

        while (command[i] != '\0') {
            // Verifica si estamos en un segmento entre comillas
            if (command[i] == '"' || command[i] == '\'') {
                if (!in_quotes) {
                    in_quotes = 1;
                    quote_char = command[i];
                } else if (command[i] == quote_char) {
                    in_quotes = 0;
                    quote_char = '\0';
                }
            }
            
            //solo considera el pipe como separador si no está entre comillas
            if (command[i] == '|' && !in_quotes) {
                command[i] = '\0';  //termina el comando actual
                
                //limpia espacios al principio y al final
                char *cmd = start;
                while (*cmd == ' ') cmd++;
                char *end = cmd + strlen(cmd) - 1;
                while (end > cmd && *end == ' ') *end-- = '\0';
                
                commands[command_count++] = cmd;
                start = &command[i + 1];  //siguiente comando comienza después del pipe
                
                if (command_count >= MAX_COMMANDS) {
                    fprintf(stderr, "Error: Too many commands\n");
                    return 1;
                }
            }
            
            i++;
        }

        //agrega el último comando
        if (*start) {
            //limpia espacios al principio y al final
            while (*start == ' ') start++;
            char *end = start + strlen(start) - 1;
            while (end > start && *end == ' ') *end-- = '\0';
            
            commands[command_count++] = start;
        }
        
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
                int argc = parse_args(commands[i], args);
                if (argc == -1) {
                    fprintf(stderr, "Error: too many arguments\n");
                    exit(EXIT_FAILURE);
                }
                
                //comando exit
                if (strcmp(args[0], "exit") == 0) {
                    exit(0);
                }
                
                //depuracion
                // fprintf(stderr, "Ejecutando comando: %s\n", args[0]);
                // fprintf(stderr, "Con argumentos:\n");
                // for (int j = 0; args[j] != NULL; j++) {
                //     fprintf(stderr, "  args[%d]: '%s'\n", j, args[j]);
                // }
                
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