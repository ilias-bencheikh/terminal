#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void print(char* string , int sortie);

int execute_kill(char** arg) {
    
    if (arg[1] == NULL) {
        print("fsh: kill: missing argument\n", STDERR_FILENO);
        return 1;
    }
    pid_t pid = atoi(arg[1]);
    int signal = (arg[2] != NULL) ? atoi(arg[2]) : SIGTERM;

    if (kill(pid, signal) < 0 ) {
        perror("kill");
        return 1;
    }
    char buffer[120];
    snprintf(buffer, sizeof(buffer), "fsh: kill: le processus %ld a été tué\n", (long)pid);
    print(buffer, STDOUT_FILENO);
    return 0;
}

