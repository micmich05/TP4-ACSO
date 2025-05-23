#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    int n, start, status, pid;
    int val;
    
    if (argc != 4) {
        printf("Uso: anillo <n> <c> <s>\n");
        exit(1);
    }
    
    n = atoi(argv[1]);
    val = atoi(argv[2]);
    start = atoi(argv[3]); // start es 1-indexado
    
    if (n < 2) {
        fprintf(stderr, "Error: se necesitan al menos 2 procesos\n");
        exit(1);
    }
    
    if (start < 1 || start > n) {
        fprintf(stderr, "Error: el proceso inicial debe estar entre 1 y %d\n", n);
        exit(1);
    }
    
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i\n",
           n, val, start);
    
    // Crear un arreglo de n pipes
    int pipes[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    
    // Cada hijo leerá de pipes[i] y escribirá a pipes[(i+1)%n]
    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {  // Proceso hijo con índice i
            // Cerrar en cada hijo los extremos que no utilizará:
            for (int j = 0; j < n; j++) {
                if (j != i)
                    close(pipes[j][0]);
                if (j != ((i + 1) % n))
                    close(pipes[j][1]);
            }
            
            int temp;
            // Leer del pipe[i]
            if (read(pipes[i][0], &temp, sizeof(int)) != sizeof(int)) {
                perror("child read");
                exit(1);
            }
            temp++;  // Incrementar el valor recibido
            
            // Escribir al pipe[(i+1)%n]
            if (write(pipes[(i + 1) % n][1], &temp, sizeof(int)) != sizeof(int)) {
                perror("child write");
                exit(1);
            }
            
            close(pipes[i][0]);
            close(pipes[(i + 1) % n][1]);
            exit(0);
        }
    }
    
    // Proceso padre: inyecta el mensaje y recoge el resultado final
    // Convertir start (1-indexado) a índice
    int start_index = start - 1;
    
    // En el padre solo necesitamos el pipe de inyección/recepción:
    // Usaremos pipes[start_index] para escribir (inyección) y leer (final)
    for (int i = 0; i < n; i++) {
        if (i != start_index) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }
    
    // Inyectar el mensaje inicial en pipes[start_index]
    if (write(pipes[start_index][1], &val, sizeof(int)) != sizeof(int)) {
        perror("parent write");
        exit(1);
    }
    close(pipes[start_index][1]);
    
    // Leer el mensaje final (después de haber circulado por todos los procesos)
    if (read(pipes[start_index][0], &val, sizeof(int)) != sizeof(int)) {
        perror("parent read");
        exit(1);
    }
    close(pipes[start_index][0]);
    
    printf("Resultado final: %d\n", val);
    
    // Esperar a que finalicen todos los procesos hijos
    for (int i = 0; i < n; i++) {
        wait(&status);
    }
    return 0;
}