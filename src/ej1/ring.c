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
    n = atoi(argv[1]);        // Número de procesos en el anillo
    buffer[0] = atoi(argv[2]);  // Valor inicial del mensaje
    start = atoi(argv[3]);      // Proceso (1-indexado) que inicia la comunicación

    if (n < 2) {
        fprintf(stderr, "Error: se necesitan al menos 2 procesos\n");
        exit(1);
    }

    // Validar que start esté dentro del rango
    if (start < 1 || start > n) {
        printf("Error: el proceso inicial debe estar entre 1 y %d\n", n);
        exit(1);
    }

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i\n",
           n, buffer[0], start);

    // Crear n pipes para la comunicación entre procesos
    int pipes[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    // Crear procesos hijos que formarán el anillo
    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            // Proceso hijo i+1 (identificados de 1 a n)
            // int my_id = i + 1;  // Si se quiere documentar el id, pero no es usado
            // (void)my_id; // Para suprimir warning

            // Cada hijo lee del pipe que tiene a su izquierda:
            // El primer hijo (i == 0) lee del pipe[n-1], el resto lee de pipe[i-1]
            int read_pipe = (i == 0) ? (n - 1) : (i - 1);
            // Cada hijo escribe en su propio pipe
            int write_pipe = i;

            // Cerrar en el hijo todos los extremos de pipes que no usará
            for (int j = 0; j < n; j++) {
                if (j != read_pipe)
                    close(pipes[j][0]);
                if (j != write_pipe)
                    close(pipes[j][1]);
            }

            // Leer mensaje del pipe asignado
            if (read(pipes[read_pipe][0], buffer, sizeof(int)) != sizeof(int)) {
                perror("child read");
                exit(1);
            }

            // Incrementar el valor recibido
            buffer[0]++;

            // Escribir al pipe asignado para enviar al siguiente en el anillo
            if (write(pipes[write_pipe][1], buffer, sizeof(int)) != sizeof(int)) {
                perror("child write");
                exit(1);
            }
            close(pipes[read_pipe][0]);
            close(pipes[write_pipe][1]);
            exit(0);
        }
    }

    // Proceso padre: se encarga de inyectar el mensaje y leer el resultado
    // Determinar el pipe de escritura (corresponde al hijo 'start')
    int start_pipe = start - 1;  
    // El pipe de lectura es el que precede al hijo 'start'
    int read_pipe = (start_pipe == 0) ? (n - 1) : (start_pipe - 1);

    // En el padre se cierran los extremos que no se usarán:
    for (int i = 0; i < n; i++) {
        if (i != start_pipe)
            close(pipes[i][1]);  // Cerrar escritura excepto la del hijo start
        if (i != read_pipe)
            close(pipes[i][0]);  // Cerrar lectura excepto la del predecesor de start
    }

    // Enviar el mensaje inicial al hijo "start"
    if (write(pipes[start_pipe][1], buffer, sizeof(int)) != sizeof(int)) {
        perror("parent write");
        exit(1);
    }
    close(pipes[start_pipe][1]);

    // Leer el mensaje final que regresa del anillo
    if (read(pipes[read_pipe][0], buffer, sizeof(int)) != sizeof(int)) {
        perror("parent read");
        exit(1);
    }
    close(pipes[read_pipe][0]);

    printf("Resultado final: %d\n", buffer[0]);

    // Esperar a que finalicen todos los procesos hijos
    for (int i = 0; i < n; i++) {
        wait(&status);
    }
    return 0;
}