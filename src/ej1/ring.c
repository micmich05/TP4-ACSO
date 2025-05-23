#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: anillo <n> <c> <s>\n");
        exit(1);
    }

    int n     = atoi(argv[1]);
    int value = atoi(argv[2]);
    int start = atoi(argv[3]);

    if (n < 1 || start < 1 || start > n) {
        fprintf(stderr, "Error: argumentos inválidos\n");
        exit(1);
    }
    printf("Se crearán %d procesos, se enviará el valor %d desde proceso %d\n",
           n, value, start);

    /* 1) Creo n tuberías */
    int pipefd[n][2];
    for (int i = 0; i < n; i++)
        if (pipe(pipefd[i]) < 0) { perror("pipe"); exit(1); }

    /* 2) Creo los n hijos */
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            /* --- código de cada hijo i (i va de 0 a n-1) --- */
            int id = i + 1;
            int r = pipefd[i][0];              // lee de su tubo
            int w = pipefd[(i + 1) % n][1];    // escribe al siguiente

            /* cierro todos los demás */
            for (int j = 0; j < n; j++) {
                if (j != i)             close(pipefd[j][0]);
                if (j != (i + 1) % n)   close(pipefd[j][1]);
            }

            /* leo, incremento, envío */
            int v;
            if (read(r, &v, sizeof(v)) != sizeof(v)) {
                perror("read hijo");
                exit(1);
            }
            printf("  [Hijo %d] recibió %d → envía %d\n", id, v, v+1);
            v++;
            if (write(w, &v, sizeof(v)) != sizeof(v)) {
                perror("write hijo");
                exit(1);
            }

            /* ¡muy importante! */
            exit(0);
        }
        /* el padre sigue creando el siguiente hijo */
    }

    /* --- código del padre --- */
    int idx = start - 1;
    /* cierro todo excepto pipefd[idx][1] (para inyectar)
       y pipefd[idx][0] (para recolectar) */
    for (int j = 0; j < n; j++) {
        if (j != idx)          close(pipefd[j][0]);
        if (j != idx)          close(pipefd[j][1]);
    }

    /* inyecto el valor inicial */
    if (write(pipefd[idx][1], &value, sizeof(value)) != sizeof(value)) {
        perror("write padre");
        exit(1);
    }
    close(pipefd[idx][1]);  // cierro escritura antes de leer

    /* leo el resultado final */
    if (read(pipefd[idx][0], &value, sizeof(value)) != sizeof(value)) {
        perror("read padre");
        exit(1);
    }
    printf("Resultado final: %d\n", value);
    close(pipefd[idx][0]);

    /* espero a todos los hijos */
    for (int i = 0; i < n; i++)
        wait(NULL);

    return 0;
}
