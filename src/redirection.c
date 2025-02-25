// redirection.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

int execute_commande(char **cmd, int last_status); 
void print(const char* string, int sortie);

int verif_redirection(char **cmd, int pos) {
    if (strcmp(cmd[pos], "<") == 0 || strcmp(cmd[pos], ">") == 0 || strcmp(cmd[pos], "2>") == 0 ||
        strcmp(cmd[pos], ">>") == 0 || strcmp(cmd[pos], "2>>") == 0 || strcmp(cmd[pos], ">|") == 0 ||
        strcmp(cmd[pos], "2>|") == 0) {
        return 1;
    }
    return 0;
}

int hasredirection(char** cmd) {
    int x = -1;
    int i = 0;
    while (cmd[i] != NULL) {
        if (verif_redirection(cmd, i) == 1) {
            x = i;
        }
        i++;
    }
    return x;
}

void extract(char **tokens, char **cmd, int pos) {
    for (int i = 0; i < pos; i++) {
        cmd[i] = tokens[i];
    }
    cmd[pos] = NULL;
}

int execute_redirection(char **tokens, int pos) {
    int flag = 0;
    int sortie_erreur = 0;
    int last_status = 0;

    // Allocation de cmd
    char **cmd = malloc((pos + 1) * sizeof(char *));
    if (cmd == NULL) {
        perror("Erreur lors de l'allocation de mémoire pour les commandes");
        return 1;
    }

    extract(tokens, cmd, pos);

    // Vérification des erreurs
    if (cmd[0] == NULL) {
        print("Aucune commande spécifiée\n", STDERR_FILENO);
        free(cmd); // Libération avant de retourner
        return 1;
    } else if (tokens[pos + 1] == NULL) {
        print("Aucun fichier spécifié\n", STDERR_FILENO);
        free(cmd); // Libération avant de retourner
        return 1;
    }

    if (strcmp(tokens[pos], "<") == 0) {
        flag = O_RDONLY;
        int fd = open(tokens[pos + 1], flag);
        if (fd == -1) {
            perror("Erreur d'ouverture du fichier");
            free(cmd); // Libération avant de retourner
            return 1;
        }

        // Sauvegarde de stdin
        int stdin_copy = dup(fileno(stdin));
        if (stdin_copy == -1) {
            perror("dup");
            close(fd);
            free(cmd);
            return 1;
        }

        // Redirection de stdin
        if (dup2(fd, fileno(stdin)) == -1) {
            perror("dup2");
            close(fd);
            close(stdin_copy);
            free(cmd);
            return 1;
        }
        close(fd);

        // Exécution de la commande
        last_status = execute_commande(cmd, last_status);

        // Restauration de stdin
        if (dup2(stdin_copy, fileno(stdin)) == -1) {
            perror("dup2");
            close(stdin_copy);
            free(cmd);
            return 1;
        }
        close(stdin_copy);

        // Libération de cmd après utilisation
        free(cmd);
        return last_status;
    } 
    else { 
        if (strcmp(tokens[pos], ">") == 0) {
            flag = O_CREAT | O_EXCL | O_WRONLY;
        } else if (strcmp(tokens[pos], "2>") == 0) {
            flag = O_CREAT | O_WRONLY | O_EXCL;
            sortie_erreur = 1;
        } else if (strcmp(tokens[pos], ">>") == 0) {
            flag = O_CREAT | O_WRONLY | O_APPEND;
        } else if (strcmp(tokens[pos], "2>>") == 0) {
            flag = O_CREAT | O_WRONLY | O_APPEND;
            sortie_erreur = 1;
        } else if (strcmp(tokens[pos], ">|") == 0) {
            flag = O_CREAT | O_WRONLY | O_TRUNC;
        } else if (strcmp(tokens[pos], "2>|") == 0) {
            flag = O_CREAT | O_WRONLY | O_TRUNC;
            sortie_erreur = 1;
        }
        else {
            print("Type de redirection inconnu\n", STDERR_FILENO);
            free(cmd); // Libération avant de retourner
            return 1;
        }

        // Sauvegarde des descripteurs
        int stdout_copy = dup(fileno(stdout));
        if (stdout_copy == -1) {
            perror("dup");
            free(cmd);
            return 1;
        }

        int stderr_copy = -1;
        if (sortie_erreur) {
            stderr_copy = dup(fileno(stderr));
            if (stderr_copy == -1) {
                perror("dup");
                close(stdout_copy);
                free(cmd);
                return 1;
            }
        }

        // Ouverture du fichier de redirection
        int fd = open(tokens[pos + 1], flag, 0664);
        if (fd == -1) {
            perror("Erreur d'ouverture du fichier");
            if (sortie_erreur) close(stderr_copy);
            close(stdout_copy);
            free(cmd);
            return 1;
        }

        // Redirection
        if (sortie_erreur == 1) {
            if (dup2(fd, fileno(stderr)) == -1) {
                perror("dup2");
                close(fd);
                close(stderr_copy);
                close(stdout_copy);
                free(cmd);
                return 1;
            }
        }
        else {
            if (dup2(fd, fileno(stdout)) == -1) {
                perror("dup2");
                close(fd);
                close(stdout_copy);
                free(cmd);
                return 1;
            }
        }
        close(fd);

        // Exécution de la commande
        last_status = execute_commande(cmd, last_status);

        // Restauration des descripteurs
        if (sortie_erreur == 1) {
            if (dup2(stderr_copy, fileno(stderr)) == -1) {
                perror("dup2");
                close(stderr_copy);
                close(stdout_copy);
                free(cmd);
                return 1;
            }
            close(stderr_copy);
        }
        else {
            if (dup2(stdout_copy, fileno(stdout)) == -1) {
                perror("dup2");
                close(stdout_copy);
                free(cmd);
                return 1;
            }
            close(stdout_copy);
        }

        free(cmd);
        return last_status;
    }
}
