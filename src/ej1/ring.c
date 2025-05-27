#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

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

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i\n",
           n, buffer[0], start);

    int total = n + 1; // padre + n hijos
    // matriz de pipes: pipes[i] conecta ring_procs[i] -> ring_procs[(i+1)%(n+1)]
    int (*pipes)[2] = malloc(sizeof(int[2]) * total);
    if (!pipes) {perror("malloc"); exit(EXIT_FAILURE); }

    for (int i = 0; i < total; ++i) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    /* Fork de los n hijos */
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
    if (pid > 0) {
        // estamos en el padre
        ring_idx = 0;
    } else {
        // estamos en un hijo myid; calculo su posición en el anillo
        // valores 1..n
        ring_idx = ((myid - start + n) % n) + 1;
    }

    /* Cierro los pipes que no me corresponden */
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

    if (ring_idx == 0) {
        /* === PADRE === */
        // envío el mensaje inicial al hijo 'start'
        if (write(pipes[0][1], &buffer[0], sizeof(buffer[0])) != sizeof(buffer[0])) {
            perror("write padre");
            exit(EXIT_FAILURE);
        }
        close(pipes[0][1]);

        // recibo el resultado final
        int result;
        if (read(pipes[total-1][0], &result, sizeof(result)) != sizeof(result)) {
            perror("read padre");
            exit(EXIT_FAILURE);
        }
        close(pipes[total-1][0]);

        printf("Resultado final: %d\n", result);

        // espero a todos los hijos para evitar zombies
        for (int i = 0; i < n; ++i) {
            wait(NULL);
        }
    } else {
        /* === HIJO === */
        int recv;
        int prev = (ring_idx - 1 + total) % total;
        if (read(pipes[prev][0], &recv, sizeof(recv)) != sizeof(recv)) {
            perror("read hijo");
            exit(EXIT_FAILURE);
        }
        close(pipes[prev][0]);

        // incremento y paso al siguiente
        recv++;
        if (write(pipes[ring_idx][1], &recv, sizeof(recv)) != sizeof(recv)) {
            perror("write hijo");
            exit(EXIT_FAILURE);
        }
        close(pipes[ring_idx][1]);

        exit(EXIT_SUCCESS);
    }

    return 0;
}
