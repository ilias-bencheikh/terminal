#include <stdio.h>       
#include <unistd.h>      
#include <string.h>      
#include <stdlib.h>      
#include <limits.h>   
#include <linux/limits.h>     

// Variable globale pour stocker le répertoire précédent
static char previous_dir[PATH_MAX] = "";

/**
 * Cette fonction change le répertoire de travail courant vers :
 * - REF : si un argument est fourni et valide
 * - Le répertoire précédent si l'argument est "-"
 * - Le répertoire HOME si aucun argument n'est fourni
 *
 * @param args Tableau de chaînes contenant les arguments de la commande
 * @return int Retourne 0 en cas de succès, 1 en cas d'échec.
 */

int verif(char *arg);
void print(const char* string , int sortie);

int execute_cd(char **args) {
    char *target;         // Répertoire cible
    char cwd[PATH_MAX];   // Buffer pour stocker le répertoire courant

    // Vérifiez que args n'est pas NULL
    if (args == NULL) {
        print("cd: arguments invalides\n", STDERR_FILENO);
        return 1;
    }

    // Obtenir le répertoire courant avant le changement
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("cd");
        return 1; 
    }

    // Comptage des arguments
    int count = 0;
    while (args[count] != NULL) {
        count++;
    }
    if (count > 2) {
        print("cd: Trop d'arguments\n", STDERR_FILENO);
        return 1;
    }

    // Déterminer la cible du changement de répertoire
    if (count == 1 || strcmp(args[1], "~") == 0 || verif(args[1]) == 0) {
        // Aucun argument fourni, utiliser $HOME
        target = getenv("HOME");
        if (target == NULL) {
            print("cd: HOME non défini\n", STDERR_FILENO);
            return 1;
        }
    }
    else if (strcmp(args[1], "-") == 0) {
        // Argument "-", revenir au répertoire précédent
        if (strlen(previous_dir) == 0) {
            print("cd: répertoire précédent non défini\n", STDERR_FILENO);
            return 1;
        }
        target = previous_dir;
    }
    else {
        // Argument fourni, utiliser REF
        target = args[1];
    }

    // Tenter de changer de répertoire
    if (chdir(target) != 0) {
        perror("cd"); 
        return 1;
    }

    // Mettre à jour le répertoire précédent
    strncpy(previous_dir, cwd, PATH_MAX - 1);
    previous_dir[PATH_MAX - 1] = '\0'; // Assurez-vous que la chaîne est terminée

    return 0; // Succès
}
