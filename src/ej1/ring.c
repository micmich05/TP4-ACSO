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
        fprintf(stderr, "Uso: anillo <n> <c> <s>\n");
        exit(1);
    }
    /* Parsing of arguments */
    n      = atoi(argv[1]);
    buffer[0] = atoi(argv[2]);
    start  = atoi(argv[3]);
    if (n < 1) {
        fprintf(stderr, "Error: n debe ser >= 1\n");
        exit(1);
    }
    if (start < 1 || start > n) {
        fprintf(stderr, "Error: s debe estar entre 1 y n\n");
        exit(1);
    }

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i\n",
           n, buffer[0], start);

    /* Create an array of n pipes */
    int (*pipefd)[2] = malloc(n * sizeof *pipefd);
    if (!pipefd) {
        perror("malloc");
        exit(1);
    }
    for (int i = 0; i < n; i++) {
        if (pipe(pipefd[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    /* Fork the n children */
    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            /* Child i */
            int id        = i + 1;
            int read_end  = pipefd[i][0];
            int write_end = pipefd[(i + 1) % n][1];

            /* Close unused fds */
            for (int j = 0; j < n; j++) {
                if (j != i)                close(pipefd[j][0]);
                if (j != (i + 1) % n)      close(pipefd[j][1]);
            }

            /* Read, increment, send on */
            int val;
            if (read(read_end, &val, sizeof(val)) != sizeof(val)) {
                perror("read");
                exit(1);
            }
            printf("Proceso %d recibió %d\n", id, val);
            val++;
            printf("Proceso %d envía %d\n", id, val);
            if (write(write_end, &val, sizeof(val)) != sizeof(val)) {
                perror("write");
                exit(1);
            }

            /* Clean up */
            close(read_end);
            close(write_end);
            exit(0);
        }
        /* Parent continues to fork next child */
    }

    /* Parent process: inject and collect */
    int idx = start - 1;
    /* Close all fds except the injection/collection pipe */
    for (int j = 0; j < n; j++) {
        if (j != idx) close(pipefd[j][0]);
        if (j != idx) close(pipefd[j][1]);
    }

    /* Send the initial value into the ring */
    if (write(pipefd[idx][1], &buffer[0], sizeof(buffer[0])) != sizeof(buffer[0])) {
        perror("write");
        exit(1);
    }
    close(pipefd[idx][1]);

    /* Receive the final result */
    int result;
    if (read(pipefd[idx][0], &result, sizeof(result)) != sizeof(result)) {
        perror("read");
        exit(1);
    }
    printf("Resultado final: %d\n", result);
    close(pipefd[idx][0]);

    /* Wait for all children to finish */
    while (wait(&status) > 0)
        ;

    free(pipefd);
    return 0;
}
