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
    n = atoi(argv[1]);
    buffer[0] = atoi(argv[2]);
    start = atoi(argv[3]);
    
    if (n < 3 || start < 1 || start > n) {
        printf("Error: n debe ser >= 3 y s debe estar entre 1 y n\n");
        exit(1);
    }
    
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
    /* Create pipes for the ring */
    int pipes[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error creando pipe");
            exit(1);
        }
    }
    
    /* Create child processes */
    for (int i = 1; i <= n; i++) {
        pid = fork();
        
        if (pid < 0) {
            perror("Error en fork");
            exit(1);
        }
        
        if (pid == 0) { // Child process
            int my_pos = i;
            int prev_pos = (my_pos == 1) ? n : my_pos - 1;
            int next_pos = (my_pos == n) ? 1 : my_pos + 1;
            
            // Pipe to read from previous process
            int read_pipe = prev_pos - 1;
            // Pipe to write to next process
            int write_pipe = my_pos - 1;
            
            // Close all unused pipes
            for (int j = 0; j < n; j++) {
                if (j != read_pipe && j != write_pipe) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            
            // Close unused ends of used pipes
            close(pipes[read_pipe][1]);  // Close write end of read pipe
            close(pipes[write_pipe][0]);  // Close read end of write pipe
            
            int value;
            read(pipes[read_pipe][0], &value, sizeof(int));
            
            printf("Proceso %d recibió %d, incrementa y envía %d\n", my_pos, value, value + 1);
            value++; // Increment the value
            
            write(pipes[write_pipe][1], &value, sizeof(int));
            
            close(pipes[read_pipe][0]);
            close(pipes[write_pipe][1]);
            exit(0);
        }
    }
    
    /* Parent process */
    // Determine which pipes to use for communication
    int write_pipe = start - 1;  // Pipe to write to starting process
    int read_pipe = start % n;   // Pipe to read from the process before starting process
    
    // Close all unused pipes
    for (int j = 0; j < n; j++) {
        if (j != write_pipe && j != read_pipe) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        } else {
            if (j == write_pipe) close(pipes[j][0]);  // Close read end of write pipe
            if (j == read_pipe) close(pipes[j][1]);   // Close write end of read pipe
        }
    }
    
    // Send the initial value to the starting process
    printf("Padre envía %d al proceso %d\n", buffer[0], start);
    write(pipes[write_pipe][1], buffer, sizeof(int));
    close(pipes[write_pipe][1]);
    
    // Read the final value after it goes around the ring
    read(pipes[read_pipe][0], buffer, sizeof(int));
    printf("Padre recibe valor final: %d\n", buffer[0]);
    close(pipes[read_pipe][0]);
    
    // Print the final result
    printf("Resultado final: %d\n", buffer[0]);
    
    // Wait for all children to finish
    for (int i = 0; i < n; i++) {
        wait(&status);
    }
    
    return 0;
}