#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 101 // 100 arguments + NULL

// int parse_args(char *cmd, char **args) {
//     int argc = 0;
//     char *p = cmd;
    
//     while (*p && argc < MAX_ARGS - 1) {  

//         while (*p == ' ' || *p == '\t') p++;
//         if (!*p) break;
        
//         if (*p == '"' || *p == '\'') {  //manejar ambas comillas
//             char quote = *p;  
//             p++; //saltar comilla inicial
//             args[argc] = p;
//             while (*p && *p != quote) p++;  //comilla de cierre
//             if (*p == quote) {
//                 *p = '\0';
//                 p++;
//             }
//         } else {
//             //caso en que el argumento no está entre comillas
//             args[argc] = p;
//             while (*p && *p != ' ' && *p != '\t') p++;
//             if (*p) {
//                 *p = '\0';
//                 p++;
//             }
//         }
//         argc++;
//     }
    
//     if (argc >= MAX_ARGS) {
//         return -1;
//     }
    
//     args[argc] = NULL;
//     return argc;
// }

// int parse_args(char *cmd, char **args) {
//     int argc = 0;
//     char *p = cmd;
    
//     while (*p && argc < MAX_ARGS - 1) {  
//         while (*p == ' ' || *p == '\t') p++;
//         if (!*p) break;
        
//         if (*p == '"' || *p == '\'') {  //manejar ambas comillas
//             char quote = *p;  
//             p++; //saltar comilla inicial
//             args[argc] = p;
            
//             // Preservar contenido dentro de comillas SIN procesar escapes
//             while (*p && *p != quote) {
//                 p++;
//             }
            
//             if (*p == quote) {
//                 *p = '\0';
//                 p++;
//             }
//         } else {
//             //caso en que el argumento no está entre comillas
//             args[argc] = p;
//             while (*p && *p != ' ' && *p != '\t') p++;
//             if (*p) {
//                 *p = '\0';
//                 p++;
//             }
//         }
//         argc++;
//     }
    
//     if (argc >= MAX_ARGS) {
//         return -1;
//     }
    
//     args[argc] = NULL;
//     return argc;
// }

// int main() {
//     char command[256];
//     char *commands[MAX_COMMANDS];
//     int command_count;

//     while (1) {
//         printf("Shell> ");
        
//         /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
//         fgets(command, sizeof(command), stdin);

//         /* Removes the newline character (\n) from the end of the string stored in command, if present. 
//            This is done by replacing the newline character with the null character ('\0').
//            The strcspn() function returns the length of the initial segment of command that consists of 
//            characters not in the string specified in the second argument ("\n" in this case). */
//         command[strcspn(command, "\n")] = '\0';
        
//          /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
//            Each resulting token is stored in the commands[] array. 
//            The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
//            In each iteration of the while loop, strtok() returns the next token found in command. 
//            The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
        
//         //modifique un poco el codigo para espacios al principio y final
//         command_count = 0;

//         char *tok = strtok(command, "|");
//         while (tok && command_count < MAX_COMMANDS) {
//             while (*tok == ' ') tok++;
//             char *end = tok + strlen(tok) - 1;
//             while (end > tok && *end == ' ') *end-- = '\0';

//             commands[command_count++] = tok;
//             tok = strtok(NULL, "|");
//         }
        
//         int N = command_count;
//         int pipes[N-1][2]; //creo los pipes necesarios (N-1 pipes para N comandos)
//         for (int i = 0; i < N-1; i++) {
//             if (pipe(pipes[i]) == -1) {
//                 perror("Error creating pipes");
//                 for (int j = 0; j < i; j++) { //cierro pipes ya fueron creados
//                     close(pipes[j][0]);
//                     close(pipes[j][1]);
//                 }
//                 exit(EXIT_FAILURE);
//             }
//         }

//         pid_t pids[N];
//         for (int i = 0; i < N; i++) {
//             pid_t pid = fork(); //creo un hijo para cada comando
//             if (pid < 0) {
//                 perror("Error executing fork");
//                 exit(EXIT_FAILURE);
//             }

//             if (pid == 0) {
//                 //redirijo la entrada/salida según corresponda
//                 if (i > 0) { //si no es el primer comando
//                     dup2(pipes[i-1][0], STDIN_FILENO);
//                 }
//                 if (i < N-1) { //si no es el último comando
//                     dup2(pipes[i][1], STDOUT_FILENO);
//                 }
//                 //cierro todas las copias de los pipes
//                 for (int j = 0; j < N-1; j++) {
//                     close(pipes[j][0]);
//                     close(pipes[j][1]);
//                 }

//                 //preparo los argumentos para execvp (la funcion parse_args maneja el caso de comillas) y 
//                 //devuelve -1 si hay demasiados argumentos
//                 char *args[MAX_ARGS];
//                 int argc = parse_args(commands[i], args);
//                 if (argc == -1) {
//                     fprintf(stderr, "Error: too many arguments\n");
//                     exit(EXIT_FAILURE);
//                 }

//                 if (strcmp(args[0], "exit") == 0) {
//                     exit(0);  // termina el proceso hijo limpiamente
//                 }

//                 // Depuración: Mostrar los argumentos pasados a execvp
//                 fprintf(stderr, "Ejecutando comando: %s\n", args[0]);
//                 fprintf(stderr, "Con argumentos:\n");
//                 for (int j = 0; args[j] != NULL; j++) {
//                     fprintf(stderr, "  args[%d]: '%s'\n", j, args[j]);
//                 }

//                 execvp(args[0], args);
//                 perror("Error executing command");
//                 exit(EXIT_FAILURE);
//             }
            
//             pids[i] = pid;
//         }

//         //cierro los pipes en el padre
//         for (int i = 0; i < N-1; i++) {
//             close(pipes[i][0]);
//             close(pipes[i][1]);
//         }

//         //espero a que terminen todos los hijos
//         for (int i = 0; i < N; i++) {
//             waitpid(pids[i], NULL, 0);
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
            
            // Preservar contenido dentro de comillas SIN procesar escapes
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
        
        /* Nuevo código que respeta las comillas al dividir por pipes */
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
            
            // Solo considera el pipe como separador si no está entre comillas
            if (command[i] == '|' && !in_quotes) {
                command[i] = '\0';  // Termina el comando actual
                
                // Limpia espacios al principio y al final
                char *cmd = start;
                while (*cmd == ' ') cmd++;
                char *end = cmd + strlen(cmd) - 1;
                while (end > cmd && *end == ' ') *end-- = '\0';
                
                commands[command_count++] = cmd;
                start = &command[i + 1];  // El siguiente comando comienza después del pipe
                
                if (command_count >= MAX_COMMANDS) {
                    fprintf(stderr, "Error: demasiados comandos\n");
                    return 1;
                }
            }
            
            i++;
        }

        // Añade el último comando
        if (*start) {
            // Limpia espacios al principio y al final
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
                
                // Verificar comando exit
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