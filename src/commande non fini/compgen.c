#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <errno.h>

void print(char* string , int sortie);

int is_executable(const char *path) {
    struct stat sb;
    return (stat(path, &sb) == 0 && sb.st_mode & S_IXUSR) ? 1 : 0;
}

// commandes internes
void list_internal_commands(char *internal_commands[]) {
    for (int i = 0; internal_commands[i] != NULL; i++) {
        print(internal_commands[i], STDOUT_FILENO);
        print("\n", STDOUT_FILENO);
    }
}

int execute_compgen(char *internal_commands[], int argc, char **argv) {
    if (argc < 2) {
        perror("Usage: compgen -c");
        return 1;
    }
    
    if (strcmp(argv[1], "-c") == 0) {
        list_internal_commands(internal_commands);
    } else {
        fprintf(stderr, "compgen: option inconnue: %s\n", argv[1]);
        perror("Erreur");
        return 1;
    }

    return 0;
}
