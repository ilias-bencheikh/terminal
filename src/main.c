#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h> // Ajout pour strerror


/* ------------------------------------------------------------------
 * Déclarations de fonctions (internes et externes)
 * ------------------------------------------------------------------ */
const char *internal_commands[] = {
     "pwd", "cd", "clear", "history", "exit", "kill", 
     "ftype", "for", "if","echos","forkbomb", NULL
};

// Fonctions internes
int execute_pwd(char **args);
int execute_cd(char **args);
void execute_clear(char **args); 
int execute_history();
int execute_ftype(char **args);
int execute_kill(char **args);
int execute_if(char **cmd);
int execute_for(char **cmd);
int execute_echos(char **args);
int execute_forkbomb(char **args);

// Fonctions de redirection
int hasredirection(char **cmd);
int execute_redirection(char **args, int pos);

// Fonctions de pipeline
int haspipeline(char **cmd);
int execute_pipeline(char **commande, int pipeline);

// Fonctions externes (définies dans excutable.c)
int execute_external_command(char **args);

// Fonctions utilitaires
void print(const char *string, int sortie);

/* ------------------------------------------------------------------
 * print() : affichage rapide
 * ------------------------------------------------------------------ */
void print(const char* string, int sortie) {
    write(sortie, string, strlen(string));
}

/* ------------------------------------------------------------------
 * AFFICHAGE DU PROMPT
 * - Affiche [SIG] si last_status == 128 + SIGINT ou SIGTERM
 * - Sinon affiche la valeur numérique
 * ------------------------------------------------------------------ */
void afficher_prompt(int last_status, char *buffer, size_t size) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        const char *prefix = "getcwd: ";
        print(prefix, STDERR_FILENO);
        print(strerror(errno), STDERR_FILENO);
        print("\n", STDERR_FILENO);
        strcpy(cwd, "?");
    }

    // Couleur : vert si 0, rouge sinon
    char *color = (last_status == 0) ? "\001\033[32m\002" : "\001\033[91m\002";
    char *reset_color = "\001\033[00m\002";

    // Prépare la chaîne pour la partie [statut]
    char status_str[16];
    if (last_status == 128 + SIGINT || last_status == 128 + SIGTERM) {
        snprintf(status_str, sizeof(status_str), "SIG");
    } else {
        snprintf(status_str, sizeof(status_str), "%d", last_status);
    }

    // On limite la longueur du chemin
    size_t max_length = 27;
    if (last_status > 99) {
        max_length = 25;
    } else if (last_status > 9) {
        max_length = 26;
    }
    char display_cwd[PATH_MAX];
    if (strlen(cwd) > (max_length - 5)) {
        snprintf(display_cwd, sizeof(display_cwd), "...%s",
                 cwd + strlen(cwd) - (max_length - 5));
    } else {
        strncpy(display_cwd, cwd, sizeof(display_cwd));
        display_cwd[sizeof(display_cwd) - 1] = '\0';
    }

    // Construire le prompt final
    snprintf(buffer, size, "%s[%s]%s%s$ ", color, status_str, reset_color, display_cwd);
}

/* ------------------------------------------------------------------
 * COMPLETION POUR READLINE
 * ------------------------------------------------------------------ */
char *init_completion(const char *text, int state) {
    static int list_index, len;
    const char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    while ((name = internal_commands[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    return NULL;
}

char **completion(const char *text, int start, int end) {
    (void)end; // éviter le warning
    if (start == 0) {
        // Compléter sur commandes internes
        return rl_completion_matches(text, init_completion);
    } else {
        // Compléter sur noms de fichiers
        return rl_completion_matches(text, rl_filename_completion_function);
    }
}

/* ------------------------------------------------------------------
 * HISTORIQUE
 * ------------------------------------------------------------------ */
int execute_history() {
    HIST_ENTRY **the_list;
    int i = 0;
    the_list = history_list(); 
    if (the_list && the_list[i+1] != NULL ) {
        for (i = 0; the_list[i+1]; i++) {
            char buffer[1024];
            int len = snprintf(buffer, sizeof(buffer), 
                               "%d  %s\n", i + history_base, the_list[i]->line);
            if (len > 0) {
                print(buffer, STDOUT_FILENO);
            }
        }
    } else {
        print("Aucune commande dans l'historique.\n", STDOUT_FILENO);
        return 1;
    }
    return 0; 
}

/* ------------------------------------------------------------------
 * execute_commande()
 * - Détermine si c'est une commande interne ou externe
 * ------------------------------------------------------------------ */
int execute_commande(char **cmd, int status) {
    int last_status = status;
    int redirection = hasredirection(cmd);
    int pipeline = haspipeline(cmd);

    if (strcmp(cmd[0], "for") == 0) {
        last_status = execute_for(cmd);
    }
    else if (strcmp(cmd[0], "if") == 0) {
        last_status = execute_if(cmd);
    }
    else if (pipeline != 0) {
        last_status = execute_pipeline(cmd, pipeline + 1);
    }
    else if (redirection != -1) {
        last_status = execute_redirection(cmd, redirection);
    }
    else if (strcmp(cmd[0], "pwd") == 0) { 
        last_status = execute_pwd(cmd);
    }
    else if (strcmp(cmd[0], "cd") == 0) {
        last_status = execute_cd(cmd);
    }
    else if (strcmp(cmd[0], "clear") == 0) {
        execute_clear(cmd);
        last_status = 0; 
    }
    else if (strcmp(cmd[0], "history") == 0) {
        last_status = execute_history();
    }
    else if (strcmp(cmd[0], "ftype") == 0) {
        last_status = execute_ftype(cmd);
    }
    else if (strcmp(cmd[0], "kill") == 0) {
        last_status = execute_kill(cmd);
    }
    else if (strcmp(cmd[0], "echos") == 0) {
        // Nouvelle commande "echos"
        last_status = execute_echos(cmd);
    }
    else if (strcmp(cmd[0], "forkbomb") == 0) {
        // Nouvelle commande "forkbomb"
        last_status = execute_forkbomb(cmd);
    }
    else if (strcmp(cmd[0], "exit") == 0) {
        int exit_val = 0;
        if (cmd[1] != NULL) {
            exit_val = atoi(cmd[1]);
            if (cmd[2] != NULL) {
                print("exit: Trop d'arguments\n", STDERR_FILENO);
                return 1;
            }
        } else {
            exit_val = last_status;
        }
        exit(exit_val);
    }
    else {
        // Commande externe
        last_status = execute_external_command(cmd);
    }
    return last_status;
}

/* ------------------------------------------------------------------
 * execute_all_commands()
 * - Gère la séparation des commandes par ; et &&
 * - Gère les accolades { }, etc.
 * ------------------------------------------------------------------ */
int execute_all_commands(char **cmds, int status) {
    int last_status = status;
    size_t commande_size = 64; 
    char **commande = malloc(commande_size * sizeof(char*));
    if (!commande) {
        print("fsh: Erreur d'allocation\n", STDERR_FILENO);
        return 1;
    }

    size_t y = 0;    
    int entre_crochet = 0;
    size_t x = 0;

    while (cmds[x] != NULL) {
        if (y >= commande_size - 1) {
            commande_size *= 2;
            char **temp = realloc(commande, commande_size * sizeof(char*));
            if (!temp) {
                print("fsh: Erreur de réallocation\n", STDERR_FILENO);
                free(commande);
                return 1;
            }
            commande = temp;
        }

        if (strcmp(cmds[x], "{") == 0) {
            entre_crochet++;
            commande[y++] = cmds[x];
        }
        else if (strcmp(cmds[x], "}") == 0) {
            if (entre_crochet <= 0) {
                print("fsh: Erreur de syntaxe : } inattendu\n", STDERR_FILENO);
                free(commande);
                return 1;
            }
            entre_crochet--;
            commande[y++] = cmds[x];
        }
        else if ((strcmp(cmds[x], ";") == 0 || strcmp(cmds[x], "&&") == 0) 
                 && entre_crochet == 0) {
            commande[y] = NULL; 
            if (commande[0] != NULL && y > 0) {
                last_status = execute_commande(commande, last_status);
                // Si l'enfant a été interrompu par Ctrl+C => last_status = 130
                if (last_status == 128 + SIGINT) {
                    break;
                }
                y = 0;
            } else {
                print("fsh: Erreur de syntaxe\n", STDERR_FILENO);
                last_status = 1;
                break;
            }
            if (strcmp(cmds[x], "&&") == 0 && last_status != 0) {
                break;
            }
        }
        else {
            commande[y++] = cmds[x];
        }
        x++;
    }

    if (y > 0) {
        commande[y] = NULL;
        last_status = execute_commande(commande, last_status);
    }

    if (entre_crochet != 0) {
        print("fsh: Erreur de syntaxe : accolades non fermées\n", STDERR_FILENO);
        free(commande);
        return 1;
    }

    free(commande);
    return last_status;
}

/* ------------------------------------------------------------------
 * DECUPAGE DE LA LIGNE EN TOKENS
 * ------------------------------------------------------------------ */
char **argument(char *line, int *num_tokens) {
    int bufsize = 64;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    if (!tokens) {
        print("fsh: allocation error\n", STDERR_FILENO);
        free(tokens);
        exit(EXIT_FAILURE);
    }

    char *token = malloc(strlen(line) + 1);
    if (!token) {
        print("fsh: allocation error\n", STDERR_FILENO);
        free(tokens);
        free(token);
        exit(EXIT_FAILURE);
    }

    int tok_pos = 0;
    int in_token = 0;

    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '\\') {
            i++;
            if (line[i] != '\0') {
                token[tok_pos++] = line[i];
            }
        }
        else if (line[i] == ' ') {
            if (in_token) {
                token[tok_pos] = '\0';
                tokens[position++] = strdup(token);
                if (!tokens[position - 1]) {
                    print("fsh: allocation error\n", STDERR_FILENO);
                    free(token);
                    free(tokens);
                    exit(EXIT_FAILURE);
                }
                tok_pos = 0;
                in_token = 0;

                if (position >= bufsize) {
                    bufsize += 64;
                    tokens = realloc(tokens, bufsize * sizeof(char*));
                    if (!tokens) {
                        print("fsh: allocation error\n", STDERR_FILENO);
                        free(token);
                        free(tokens);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        else {
            token[tok_pos++] = line[i];
            in_token = 1;
        }
    }

    if (in_token) {
        token[tok_pos] = '\0';
        tokens[position++] = strdup(token);
    }

    tokens[position] = NULL;
    *num_tokens = position;
    free(token);
    return tokens;
}

/* ------------------------------------------------------------------
 * Libération des tokens
 * ------------------------------------------------------------------ */
void free_tokens(char **tokens) {
    if (tokens) {
        for (int i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }
}

/* ------------------------------------------------------------------
 * MAIN
 * - On ignore SIGINT, SIGTSTP, SIGTERM dans le parent
 *   (pour que seul l'enfant reçoive le Ctrl+C)
 * ------------------------------------------------------------------ */
int main() {
    char *ligne = NULL;
    int last_status = 0;
    char prompt[1024];
    rl_outstream = stderr;

    // Ignorer Ctrl+C, Ctrl+Z, SIGTERM dans le shell parent
    struct sigaction sa_ign;
    memset(&sa_ign, 0, sizeof(sa_ign));
    sa_ign.sa_handler = SIG_IGN;
    sigemptyset(&sa_ign.sa_mask);
    sa_ign.sa_flags = 0;
    sigaction(SIGINT, &sa_ign, NULL);
    sigaction(SIGTSTP, &sa_ign, NULL);
    sigaction(SIGTERM, &sa_ign, NULL);

    // Fonction de complétion
    rl_attempted_completion_function = completion;

    while (1) {
        afficher_prompt(last_status, prompt, sizeof(prompt));
        ligne = readline(prompt);

        if (!ligne) { 
            // EOF (Ctrl+D)
            print("\n", STDOUT_FILENO);
            break;
        }

        if (strlen(ligne) > 0) {
            add_history(ligne);
            char *line_copy = strdup(ligne);
            if (!line_copy) {
                print("strdup: ", STDERR_FILENO);
                print(strerror(errno), STDERR_FILENO);
                print("\n", STDERR_FILENO);
                free(ligne);
                continue;
            }

            int num_tokens = 0;
            char **tokens = argument(line_copy, &num_tokens);
            if (!tokens) {
                print("fsh: Erreur d'allocation\n", STDERR_FILENO);
                free(line_copy);
                free(ligne);
                exit(EXIT_FAILURE);
            }

            // Exécuter la (les) commande(s)
            last_status = execute_all_commands(tokens, last_status);

            free_tokens(tokens);
            free(line_copy);
        }
        free(ligne);
    }
    return last_status;
}
