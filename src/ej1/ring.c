#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{	
    int start, status, pid, n;
    int buffer[1];

    if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
    /* Parsing of arguments */
    n = atoi(argv[1]);        // número de procesos
    buffer[0] = atoi(argv[2]); // valor inicial del mensaje
    start = atoi(argv[3]);     // proceso que inicia la comunicación
    
    // Validar que start esté dentro del rango válido
    if (start < 1 || start > n) {
        printf("Error: el proceso inicial debe estar entre 1 y %d\n", n);
        exit(1);
    }
    
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
    /* You should start programming from here... */
    
    // Crear n pipes para la comunicación entre procesos
    int pipes[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    
    // Crear procesos hijos
    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        
        if (pid == 0) {
            // Proceso hijo i+1 (numerados de 1 a n)
            int my_id = i + 1;
            
            // Determinar de qué pipe leer y en cuál escribir
            int read_pipe = (i == 0) ? (n - 1) : (i - 1); // El primer proceso lee del último pipe
            int write_pipe = i;  // Escribo en mi pipe
            
            // Cerrar todos los extremos de pipe no utilizados
            for (int j = 0; j < n; j++) {
                if (j != read_pipe) {
                    close(pipes[j][0]); // Cerrar lectura
                }
                if (j != write_pipe) {
                    close(pipes[j][1]); // Cerrar escritura
                }
            }
            
            // Leer mensaje
            read(pipes[read_pipe][0], buffer, sizeof(int));
            
            // Incrementar el valor
            buffer[0]++;
            
            // Escribir al siguiente
            write(pipes[write_pipe][1], buffer, sizeof(int));
            
            // Cerrar los pipes utilizados
            close(pipes[read_pipe][0]);
            close(pipes[write_pipe][1]);
            
            exit(0);
        }
    }
    
    // Proceso padre - cerrar todos los pipes excepto los necesarios
    int start_pipe = start - 1; // Pipe al que escribir (proceso inicial)
    int read_pipe = (start_pipe == 0) ? (n - 1) : (start_pipe - 1); // Pipe del que leer después del recorrido
    
    for (int i = 0; i < n; i++) {
        if (i != start_pipe) {
            close(pipes[i][1]); // Cerrar escritura
        }
        if (i != read_pipe) {
            close(pipes[i][0]); // Cerrar lectura
        }
    }
    
    // Enviar mensaje inicial
    write(pipes[start_pipe][1], buffer, sizeof(int));
    close(pipes[start_pipe][1]);
    
    // Leer resultado final
    read(pipes[read_pipe][0], buffer, sizeof(int));
    close(pipes[read_pipe][0]);
    
    printf("Resultado final: %d\n", buffer[0]);
    
    // Esperar a que terminen todos los procesos hijos
    for (int i = 0; i < n; i++) {
        wait(&status);
    }
    
    return 0;
}