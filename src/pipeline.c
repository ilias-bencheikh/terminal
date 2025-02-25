#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

/**
 * Fonction qui crée un pipeline de commandes.
 * 
 * @param cmds Tableau de commandes (chaque commande est un tableau de chaînes).
 * @param n Nombre de commandes dans le pipeline.
 * @return int Retourne le code de retour de la dernière commande du pipeline.
 */

int execute_commande(char **cmd,int last_status) ;
int free_tokens(char **tokens);
int haspipeline(char **cmd) {
    int i = 0;
    int compt = 0;
    while (cmd[i] != NULL) {
        if (strcmp(cmd[i], "|") == 0) {
            compt++;
        }
        i++;
    }
    return compt;
}

void extract_cmd(char **tokens, char ***cmds) {
    int i = 0;
    int j = 0;
    int k = 0;

    while (tokens[i] != NULL) {
        if (strcmp(tokens[i], "|") == 0) {
            cmds[k][j] = NULL;
            k++;
            j = 0;
        } else {
            cmds[k][j] = tokens[i];
            j++;
        }
        i++;
    }
    cmds[k][j] = NULL;
}

int execute_pipeline(char **tokens, int n) {
    int pipefd[2];
    pid_t pid;
    int in_fd = 0;  // Descripteur d'entrée (initialement stdin)
    int status;

    char ***cmds = malloc((n + 1) * sizeof(char **));
    if (cmds == NULL) {
        perror("Erreur lors de l'allocation de mémoire pour les commandes");
        return 1;
    }

    for (int i = 0; i < n ; i++) {
        cmds[i] = malloc(64 * sizeof(char *));
        if (cmds[i] == NULL) {
            perror("Erreur lors de l'allocation de mémoire pour les commandes");
            return 1;
        }
    }

    cmds[n] = NULL;

    extract_cmd(tokens, cmds);

    int i = 0;
    while (cmds[i]!=NULL && cmds[i][0] != NULL) {
        // Crée un pipe pour chaque commande sauf la dernière
        if (i < n - 1 && pipe(pipefd) == -1) {
            perror("Erreur lors de la création du pipe");
            return 1;
        }
        pid = fork();
        if (pid == -1) {
            perror("Erreur lors du fork");
            return 1;
        }

        if (pid == 0) {
            // Redirection de l'entrée de la commande
            if (in_fd != 0) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // Redirection de la sortie de la commande, sauf pour la dernière
            if (i < n - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            } 

            // Exécution de la commande
            int s = execute_commande(cmds[i],status);
            if (s == -1) {
                perror("Erreur lors de l'exécution de la commande");
                exit(EXIT_FAILURE);
            }
            exit(s);
        } else {
            // Ferme les descripteurs inutilisés dans le processus parent
            if (in_fd != 0) close(in_fd);
            if (i < n - 1) close(pipefd[1]);

            // L'entrée pour la prochaine commande sera la lecture du pipe actuel
            in_fd = pipefd[0];
        }
        i++;
    }

    // Attendre la fin du dernier processus et récupérer son code de retour
    waitpid(pid, &status, 0);

    for(int i = 0; i < n; i++) {
        free(cmds[i]);
    }
    free(cmds);

    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}