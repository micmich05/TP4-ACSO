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
    
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
    /* You should start programming from here... */
    
    // Crear n+1 pipes (n para los procesos + 1 para que el último devuelva al padre)
    int pipes[n+1][2];
    for (int i = 0; i <= n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    
    // Crear procesos hijos
    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid == 0) {
            // Proceso hijo i
            int read_from = i;
            int write_to = (i + 1) % (n + 1);
            
            // Cerrar pipes no utilizados
            for (int j = 0; j <= n; j++) {
                if (j != read_from) close(pipes[j][0]);
                if (j != write_to) close(pipes[j][1]);
            }
            
            // Leer mensaje
            read(pipes[read_from][0], buffer, sizeof(int));
            buffer[0]++; // Incrementar el mensaje
            
            // Escribir al siguiente
            write(pipes[write_to][1], buffer, sizeof(int));
            
            close(pipes[read_from][0]);
            close(pipes[write_to][1]);
            
            exit(0);
        }
    }
    
    // Proceso padre
    // Cerrar extremos no utilizados
    for (int i = 0; i <= n; i++) {
        close(pipes[i][0]);
    }
    
    // Enviar mensaje inicial al proceso start
    write(pipes[start][1], buffer, sizeof(int));
    
    // Cerrar extremos de escritura
    for (int i = 0; i <= n; i++) {
        close(pipes[i][1]);
    }
    
    // Leer resultado final del pipe n (donde el último proceso escribe)
    int result;
    read(pipes[n][0], &result, sizeof(int));
    
    printf("Resultado final: %d\n", result);
    
    // Esperar a que todos los hijos terminen
    for (int i = 0; i < n; i++) {
        wait(&status);
    }
    
    // Cerrar pipes restantes
    close(pipes[n][0]);
    
    return 0;
}
