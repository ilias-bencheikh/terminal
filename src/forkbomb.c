#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void print(char* string , int sortie);


/**
 * forkbomb : lance une bombe de forks 
 * Tant que ça peut, ça fork. Peut geler/crasher la machine.
 */
int execute_forkbomb(char **args) {
    (void)args; 
    print("Attention: Fork Bomb en cours... Votre système risque de geler.\n", STDOUT_FILENO);

    // Boucle infinie => fork() en boucle
    while (1) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("forkbomb");
            break; 
        }
    }
    return 0; 
}
