#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print(char* string , int sortie);

/**
 * echos : Affiche tous les arguments entourés de "⭐" au début et à la fin,
 *         comme un echo custom.
 * Exemple : "echos bonjour le monde"
 *   => ⭐ bonjour le monde ⭐
 */
int execute_echos(char **args) {
    if (args[1] == NULL) {
        print("⭐  ⭐\n", STDOUT_FILENO);
        return 0;
    }

    // taille totale nécessaire
    size_t buffer_size = 4; // pour "⭐ " + " ⭐\n"
    for (int i = 1; args[i] != NULL; i++) {
        buffer_size += strlen(args[i]) + 1;
    }

    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("echos");
        return 1;
    }

    //  chaîne finale
    strcpy(buffer, "⭐ ");
    for (int i = 1; args[i] != NULL; i++) {
        strcat(buffer, args[i]);
        if (args[i+1] != NULL) {
            strcat(buffer, " ");
        }
    }
    strcat(buffer, " ⭐\n");

    print(buffer, STDOUT_FILENO);
    free(buffer);
    return 0;
}
