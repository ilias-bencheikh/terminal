#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <errno.h> // Pour gérer EINTR

struct stat *path_stat;
void print(char* string , int sortie);


char *concat(char *s1, char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    if (!result) return NULL;
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int verif(char *arg) {
    char l = arg[0];
    if (l=='<' || l=='>' || l=='|' || l=='&' || l==';' || l=='{' || l=='}' 
        || l=='$' || (l=='2' && arg[1]=='>')) {
        return 0;
    }
    return 1;
}

// Nombre de fichiers/arguments passés après la commande
int nb_arguments(char **args) {
    int compt = 0;
    int i = 1;
    while (args[i] != NULL) {
        i++;
        compt++;
    }
    return compt;
}

/* ------------------------------------------------------------------
 * execute_external_command()
 *  - Lance un fork
 *  - Dans le fils, rétablit SIG_DFL pour SIGINT, SIGQUIT, etc.
 *  - Exécute la commande via execvp
 *  - Dans le père, on attend avec waitpid(), 
 *    et on relance si waitpid() est interrompu (errno == EINTR)
 * ------------------------------------------------------------------ */
int execute_external_command(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork(); // Créer un processus enfant
    if (pid < 0) {
        perror("fsh");
        return 1; 
    }

    if (pid == 0) {
        // Rétablir le comportement par défaut pour certains signaux
        struct sigaction sa_dfl;
        memset(&sa_dfl, 0, sizeof(sa_dfl));
        sa_dfl.sa_handler = SIG_DFL;
        sigemptyset(&sa_dfl.sa_mask);
        sa_dfl.sa_flags = 0;

        sigaction(SIGINT, &sa_dfl, NULL);
        sigaction(SIGTSTP, &sa_dfl, NULL);
        sigaction(SIGQUIT, &sa_dfl, NULL);
        sigaction(SIGTERM, &sa_dfl, NULL);

        // Lancer la commande
        if (execvp(args[0], args) == -1) {
            if (access(args[0], X_OK) == -1) {
                print("fsh: ", STDERR_FILENO);
                print(args[0], STDERR_FILENO);
                print(": commande introuvable\n", STDERR_FILENO);
                exit(EXIT_FAILURE);
            } else {
                perror("fsh");
                exit(EXIT_FAILURE);
            }
        }
        // On ne revient jamais ici si execvp réussit
    }
    else {
        // -------- PROCESSUS PARENT --------
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                if (errno == EINTR) {
                    continue;
                }
                // Autre erreur
                perror("fsh");
                status = 1;
                break;
            }
            // Si le fils est stoppé (Ctrl+Z), on peut afficher un msg
            if (WIFSTOPPED(status)) {
                const char *msg = "Processus suspendu\n";
                write(STDERR_FILENO, msg, strlen(msg));
                break;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    // Interpréter le code de retour final
    if (WIFEXITED(status)) {
        status = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status)) {
        status = 128 + WTERMSIG(status);
    } 
    else {
        status = 148;
    }

    return status;
}
