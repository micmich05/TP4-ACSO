#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: anillo <n> <c> <s>\n");
        return 1;
    }

    int n     = atoi(argv[1]);
    int value = atoi(argv[2]);
    int start = atoi(argv[3]);

    if (n < 1 || start < 1 || start > n) {
        fprintf(stderr, "Argumentos inválidos (n>=1, 1<=s<=n)\n");
        return 1;
    }

    printf("Se crearán %d procesos, se enviará el valor %d desde proceso %d\n",
           n, value, start);

    // 1) Crear dinámicamente las n tuberías
    int (*p)[2] = malloc(n * sizeof *p);
    if (!p) {
        perror("malloc");
        return 1;
    }
    for (int i = 0; i < n; i++) {
        if (pipe(p[i]) < 0) {
            perror("pipe");
            return 1;
        }
    }

    // 2) Fork de los n hijos
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            // --- código del hijo i (identificado como i+1) ---
            int rd = p[i][0];                // lee de p[i]
            int wr = p[(i+1) % n][1];        // escribe en p[i+1]

            // Cerrar todo descriptor que no use este hijo
            for (int j = 0; j < n; j++) {
                if (p[j][0] != rd) close(p[j][0]);
                if (p[j][1] != wr) close(p[j][1]);
            }

            int x;
            if (read(rd, &x, sizeof x) != sizeof x) {
                perror("read hijo");
                _exit(1);
            }
            printf("[Hijo %d] recibió %d → envía %d\n", i+1, x, x+1);
            x++;
            if (write(wr, &x, sizeof x) != sizeof x) {
                perror("write hijo");
                _exit(1);
            }

            // Cerrar los que acaba de usar y salir
            close(rd);
            close(wr);
            _exit(0);
        }
        // el padre sigue creando hijos
    }

    // --- código del padre ---
    int idx = start - 1;  // convertir a índice 0…n-1

    // Cerrar todos los extremos salvo los de inyección (p[idx][1])
    // y recolección (p[idx][0])
    for (int j = 0; j < n; j++) {
        if (j != idx) {
            close(p[j][0]);
            close(p[j][1]);
        }
    }

    // Inyectar el valor inicial y cerrar el descriptor de escritura
    if (write(p[idx][1], &value, sizeof value) != sizeof value) {
        perror("write padre");
        return 1;
    }
    close(p[idx][1]);

    // Leer el resultado final (bloquea hasta que llegue EOF o dato)
    if (read(p[idx][0], &value, sizeof value) != sizeof value) {
        perror("read padre");
        return 1;
    }
    printf("Resultado final: %d\n", value);
    close(p[idx][0]);

    // Esperar a todos los hijos
    while (wait(NULL) > 0) { }

    free(p);
    return 0;
}
