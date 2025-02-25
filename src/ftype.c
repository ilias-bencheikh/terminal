#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

/**
 * Cette fonction affiche le type du fichier de référence REF.
 *
 * @param ref Le chemin vers le fichier de référence.
 * @return int Retourne 0 en cas de succès, 1 en cas d'échec.
 */

void print(const char* string , int sortie);

int execute_ftype(char **args) {
    const char *ref = args[1];
    struct stat st;


    // Vérification du fichier grâce à un lstat
    if (lstat(ref, &st) == -1) {
        perror("lstat :");
        return 1;
    }

    // Détermine et affiche le type du fichier
    const char *type = NULL;
    if (S_ISDIR(st.st_mode)) {
        type = "directory\n";
    } else if (S_ISREG(st.st_mode)) {
        type = "regular file\n";
    } else if (S_ISLNK(st.st_mode)) {
        type = "symbolic link\n";
    } else if (S_ISFIFO(st.st_mode)) {
        type = "named pipe\n";
    } else {
        type = "other\n";
    }

    // Écrit le type du fichier dans stdout
    print(type, STDOUT_FILENO);
    return 0;
}

