#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PROCESSES 123
#define MAX_C 2000000

int main(int argc, char **argv)
{
    int start, status, pid, n;
    int buffer[1];

    if (argc != 4) {
        printf("Uso: anillo <n> <c> <s>\n");
        exit(0);
    }
    /* Parsing de argumentos */
    n        = atoi(argv[1]);   // número de hijos
    buffer[0] = atoi(argv[2]);  // valor inicial a transmitir
    start    = atoi(argv[3]);   // índice del hijo que arranca

    if (n > MAX_PROCESSES) {
        printf("Número de procesos excede el máximo permitido (%d)\n", MAX_PROCESSES);
        exit(EXIT_FAILURE);
    }

    if (buffer[0] > MAX_C) {
        printf("Valor de c debe ser menor o igual a %d\n", MAX_C);
        exit(EXIT_FAILURE);
    }

    if (n < 3 || start > n || start <= 0) {
        printf("Entradas inválidas\n");
        exit(EXIT_FAILURE);
    }

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i\n",
           n, buffer[0], start);

    int total = n + 1; // padre + n hijos    
    int pipes[total][2];    
    for (int i = 0; i < total; ++i) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    //forkeo hijos
    int myid = -1;
    for (int i = 0; i < n; ++i) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        
        if (pid == 0) {
            myid = i;  // en el hijo, guardo su índice
            break;
        }
        // el padre sigue creando más hijos
    }
    
    int ring_idx;
    if (pid > 0) { //para el padre
        ring_idx = 0;
    } else { //para los hijos
        ring_idx = ((myid - (start - 1) + n) % n) + 1; 
    }

    //cierro pipes que no necesito
    for (int i = 0; i < total; ++i) {
        if (i == ring_idx) {
            // mi pipe de salida: me quedo con [1], cierro [0]
            close(pipes[i][0]);
        } else if (i == (ring_idx - 1 + total) % total) {
            // mi pipe de entrada: me quedo con [0], cierro [1]
            close(pipes[i][1]);
        } else {
            // cierro ambos extremos
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }

    if (ring_idx == 0) { //padre
        write(pipes[0][1], &buffer[0], sizeof(buffer[0]));
        close(pipes[0][1]);

        int result; // recibo el resultado final
        read(pipes[total-1][0], &result, sizeof(result));
        close(pipes[total-1][0]);

        printf("Resultado final: %d\n", result);

        for (int i = 0; i < n; ++i) { //espero a todos los hijos
            wait(NULL);
        }
        
    } else { //hijos
        int recv;
        int prev = (ring_idx - 1 + total) % total;

        read(pipes[prev][0], &recv, sizeof(recv));
        close(pipes[prev][0]);

        recv++;  // incremento y paso al siguiente
        write(pipes[ring_idx][1], &recv, sizeof(recv));
        close(pipes[ring_idx][1]);

        exit(EXIT_SUCCESS);
    }

    return 0;
}