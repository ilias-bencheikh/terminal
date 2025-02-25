#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>


/**
 * Cette fonction affiche le répertoire de travail courant absolu.
 *
 * @return int Retourne 0 en cas de succès, 1 en cas d'échec.
 */

void print(char* string , int sortie);

int execute_pwd(char **args) {
    char cwd[PATH_MAX]; // Buffer pour stocker le répertoire courant
    if (args[1] != NULL) {
        print("pwd: Trop d'arguments\n", STDERR_FILENO);
        return 1;
    }
    // Obtenir le répertoire de travail actuel
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        print(cwd, STDOUT_FILENO);
        print("\n", STDOUT_FILENO);
        return 0;            
    } else {
        perror("fsh: pwd");
        return 1;            
    }
}
