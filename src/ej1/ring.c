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
            // Each process reads from previous and writes to next
            int read_pipe = (i == 1) ? n - 1 : i - 2;
            int write_pipe = i - 1;
            
            // Close all unused pipes
            for (int j = 0; j < n; j++) {
                if (j != read_pipe && j != write_pipe) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            
            // Close unused ends of used pipes
            close(pipes[read_pipe][1]);
            close(pipes[write_pipe][0]);
            
            int value;
            read(pipes[read_pipe][0], &value, sizeof(int));
            
            value++; // Increment the value
            
            write(pipes[write_pipe][1], &value, sizeof(int));
            
            close(pipes[read_pipe][0]);
            close(pipes[write_pipe][1]);
            exit(0);
        }
    }
    
    /* Parent process */
    // Set up communication with the ring
    int write_to_start = start - 1;
    int read_from_last = (start == 1) ? n - 1 : start - 2;
    
    // Close all unused pipes
    for (int j = 0; j < n; j++) {
        if (j != write_to_start && j != read_from_last) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        } else {
            if (j == write_to_start) close(pipes[j][0]);
            if (j == read_from_last) close(pipes[j][1]);
        }
    }
    
    // Send the initial value to the starting process
    write(pipes[write_to_start][1], buffer, sizeof(int));
    close(pipes[write_to_start][1]);
    
    // Read the final value after it goes around the ring
    read(pipes[read_from_last][0], buffer, sizeof(int));
    close(pipes[read_from_last][0]);
    
    // Print the final result
    printf("Valor final después de recorrer el anillo: %d\n", buffer[0]);
    
    // Wait for all children to finish
    for (int i = 0; i < n; i++) {
        wait(&status);
    }
    
    return 0;
}