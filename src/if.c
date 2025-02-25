#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

// Prototypes des fonctions existantes
extern int execute_all_commands(char **cmds,int status);
extern char **argument(char *line, int *num_tokens);


// fonction pour gerer les commande dans les { }
int parse_block(char **cmd, int start, char ***block_cmd) {
    if (cmd[start] == NULL || strcmp(cmd[start], "{") != 0) {
        const char *error_msg = "fsh : if : Erreur de syntaxe \n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return -1;
    }

    int brace_count = 1;
    int end = start + 1;

    // position cd '{'
    while (cmd[end] != NULL && brace_count > 0) {
        if (strcmp(cmd[end], "{") == 0) {
            brace_count++;
        } else if (strcmp(cmd[end], "}") == 0) {
            brace_count--;
        }
        end++;
    }

    if (brace_count != 0) {
        const char *error_msg = "fsh : if : Erreur de syntaxe \n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return -1;
    }

    // Calculer la taille du bloc de commandes
    int block_size = end - start - 2; // Exclure les accolades
    if (block_size < 0) {
        const char *error_msg = "fsh : if : Erreur de syntaxe \n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return -1;
    }

    *block_cmd = malloc((block_size + 1) * sizeof(char*));
    if (!(*block_cmd)) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < block_size; i++) {
        (*block_cmd)[i] = strdup(cmd[start + 1 + i]);
        if (!(*block_cmd)[i]) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
    }
    (*block_cmd)[block_size] = NULL;

    return end;
}

// exécuter if
int execute_if(char **cmd) {
    int last_status = 0;
    if (cmd[1] == NULL) {
        const char *error_msg = "fsh : if : Erreur de syntaxe \n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return 1;
    }

    // Trouver '{' pour le bloc 'if'
    int test_start = 1;
    int test_end = -1;
    for (int i = test_start; cmd[i] != NULL; i++) {
        if (strcmp(cmd[i], "{") == 0) {
            test_end = i;
            break;
        }
    }

    // si '{' est présent
    if (test_end == -1) {
        const char *error_msg = "fsh : if : Erreur de syntaxe \n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return 1;
    }

    // commande TEST    
    int test_size = test_end - test_start;
    if (test_size < 1) {
        const char *error_msg = "fsh : if : Erreur de syntaxe \n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return 2;
    }

    char **test_cmd = malloc((test_size + 1) * sizeof(char*));
    if (!test_cmd) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < test_size; i++) {
        test_cmd[i] = strdup(cmd[test_start + i]);
        if (!test_cmd[i]) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
    }
    test_cmd[test_size] = NULL;

    // pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        // vide la mémoire
        for (int i = 0; i < test_size; i++) {
            free(test_cmd[i]);
        }
        free(test_cmd);
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        // vide la mémoire
        for (int i = 0; i < test_size; i++) {
            free(test_cmd[i]);
        }
        free(test_cmd);
        return 1;
    }

    if (pid == 0) {
        // enfant
        close(pipefd[0]); // Fermer le côté lecture du pipe

        // Exécuter la commande TEST
        int test_status = execute_all_commands(test_cmd,0);

        // Écrire le statut dans le pipe
        if (write(pipefd[1], &test_status, sizeof(test_status)) == -1) {
            perror("write");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]);
        // vide la mémoire
        for (int i = 0; i < test_size; i++) {
            free(test_cmd[i]);
        }
        free(test_cmd);
        exit(EXIT_SUCCESS);
    } else {
        //  parent
        close(pipefd[1]); 

        int test_status = 0;
        ssize_t bytes_read = read(pipefd[0], &test_status, sizeof(test_status));
        if (bytes_read == -1) {
            perror("read");
            close(pipefd[0]);
            // vide la mémoire
            for (int i = 0; i < test_size; i++) {
                free(test_cmd[i]);
            }
            free(test_cmd);
            return 1;
        } else if (bytes_read == 0) {
            const char *error_msg = "fsh : if : Erreur de syntaxe \n";
            write(STDERR_FILENO, error_msg, strlen(error_msg));
            close(pipefd[0]);
            // vide la mémoire
            for (int i = 0; i < test_size; i++) {
                free(test_cmd[i]);
            }
            free(test_cmd);
            return 1;
        }

        close(pipefd[0]);

        // attendre la fin du processus enfant
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            for (int i = 0; i < test_size; i++) {
                free(test_cmd[i]);
            }
            free(test_cmd);
            return 1;
        }

        // vide la mémoire
        for (int i = 0; i < test_size; i++) {
            free(test_cmd[i]);
        }
        free(test_cmd);

        // si else 
        int else_start = -1;
        for (int i = test_end; cmd[i] != NULL; i++) {
            if (strcmp(cmd[i], "else") == 0) {
                else_start = i;
                break;
            }
        }

        // if ou else 
        if (test_status == 0) {
            // Exécuter le bloc 'if'
            char **if_block = NULL;
            int block_end = parse_block(cmd, test_end, &if_block);
            if (block_end == -1) {
                return 1;
            }
            
            last_status = execute_all_commands(if_block,0);

            // vide la mémoire
            for (int i = 0; if_block[i] != NULL; i++) {
                free(if_block[i]);
            }
            free(if_block);

        } else if (else_start != -1) { // else 
            char **else_block = NULL;
            int block_end = parse_block(cmd, else_start + 1, &else_block);
            if (block_end == -1) {
                return 1;
            }

            last_status = execute_all_commands(else_block,0);

            // vide la mémoire
            for (int i = 0; else_block[i] != NULL; i++) {
                free(else_block[i]);
            }
            free(else_block);
        }

        return last_status;
    }
}
