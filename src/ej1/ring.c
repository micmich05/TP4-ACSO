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

    /* 1) Crear n pipes */
    int pipefd[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipefd[i]) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    /* 2) Fork de los n hijos */
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            /* --- código de cada hijo i (i = 0…n-1) --- */
            int id       = i + 1;
            int read_fd  = pipefd[i][0];            // leer de su pipe
            int write_fd = pipefd[(i + 1) % n][1];  // escribir al siguiente

            /* cerrar todos los extremos que no use este hijo */
            for (int j = 0; j < n; j++) {
                if (j != i)           close(pipefd[j][0]);
                if (j != (i + 1) % n) close(pipefd[j][1]);
            }

            /* leer, incrementar, enviar */
            int v;
            if (read(read_fd, &v, sizeof(v)) != sizeof(v)) {
                perror("read hijo");
                exit(1);
            }
            printf("[Hijo %d] recibió %d → envía %d\n", id, v, v + 1);
            v++;
            if (write(write_fd, &v, sizeof(v)) != sizeof(v)) {
                perror("write hijo");
                exit(1);
            }

            /* al hacer exit() se cierran read_fd y write_fd */
            exit(0);
        }
        /* el padre sigue creando los siguientes hijos */
    }

    /* --- código del padre --- */
    int idx = start - 1;
    /* cerrar todos los extremos salvo los de inyección y recolección */
    for (int j = 0; j < n; j++) {
        if (j != idx)       close(pipefd[j][0]);
        if (j != idx)       close(pipefd[j][1]);
    }

    /* inyectar el valor inicial */
    if (write(pipefd[idx][1], &value, sizeof(value)) != sizeof(value)) {
        perror("write padre");
        exit(1);
    }
    close(pipefd[idx][1]);  // cerrar el lado de escritura para que el último hijo vea EOF

    /* leer el resultado final */
    if (read(pipefd[idx][0], &value, sizeof(value)) != sizeof(value)) {
        perror("read padre");
        exit(1);
    }
    printf("Resultado final: %d\n", value);
    close(pipefd[idx][0]);

    /* esperar a todos los hijos */
    while (wait(NULL) > 0)
        ;

    return 0;
}
