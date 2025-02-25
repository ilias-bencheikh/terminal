#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int execute_all_commands(char **cmds, int status);
char *concat(const char *s1, const char *s2);
void print(const char* string, int sortie);
void handle_sigint();

// profondeur maximale pour éviter les récursions infinies
#define MAX_DEPTH 100

int length(char **cmd) {
    int i = 0;
    while (cmd[i] != NULL) {
        i++;
    }
    return i;
}

int *pos_indice(char **cmd, const char *indice) {
    int *tab = malloc(sizeof(int) * 50);
    if (tab == NULL) {
        print("fsh: for: Erreur d'allocation\n", STDERR_FILENO);
        return NULL;
    }
    for(int i = 0; i < 50; i++) {
        tab[i] = -1;
    }
    int i = 0;
    int j = 0;
    while (cmd[i] != NULL) {
        if (strcmp(cmd[i], indice) == 0) {
            tab[j] = i;
            j++;
        }
        else {
            const char *tmp = cmd[i];
            int k = 0;
            while (tmp[k] != '\0') {
                if (tmp[k] == '$') {
                    if(tmp[k+1] == indice[1]){
                       tab[j] = i;
                       j++;
                       break;
                    }
                }
                k++;
            }
        }
        i++;
    }
    return tab;
}

int nb_occurence(const char *c, const char *indice) {
    int i = 0;
    int j = 0;
    while (c[i] != '\0') {
        if (c[i] == indice[0] && c[i + 1] == indice[1]) {
                j++;
        }
        i++;
    }
    return j;
}

char *remplace_variable(const char *c, const char *valeur, const char *variable){
    int occurrences = nb_occurence(c, variable);
    size_t new_length = strlen(c) + occurrences * strlen(valeur) + 1;
    char *tmp = malloc(new_length);
    if(tmp == NULL){
        print("fsh: for: Erreur d'allocation\n", STDERR_FILENO);
        return NULL;
    }
    size_t i = 0, j = 0;
    while(c[i] != '\0'){
        if(c[i] == variable[0] && c[i+1] == variable[1]){
            strcpy(&tmp[j], valeur);
            j += strlen(valeur);
            i += 2; // Sauter la variable
        }
        else{
            tmp[j++] = c[i++];
        }
    }
    tmp[j] = '\0';
    return tmp;
}

int parcours_recursif(char *directory, char **cmd, char **commande, int *parametre, char *extension, char *type, char *variable, int *indice_variable, int last_status, int sauvegarde, int depth , int parallel) {
    pid_t pid_enfants[parallel]; 
    int max_parallel = parallel;
    if (depth > MAX_DEPTH) {
        print("fsh: for: Profondeur de récursion maximale atteinte\n", STDERR_FILENO);
        return last_status;
    }

    DIR *dir = opendir(directory);
    if(dir == NULL){
        perror("opendir");
        return last_status;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if ((parametre[0] == 1 || entry->d_name[0] != '.')) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            
            char *dir_slash = concat(directory, "/");
            if (dir_slash == NULL) {
                closedir(dir);
                return last_status;
            }
            char *p = concat(dir_slash, entry->d_name);
            free(dir_slash); 
            if (p == NULL) {
                closedir(dir);
                return last_status;
            }

            struct stat st;
            if(lstat(p, &st) == -1){
                perror("lstat");
                free(p);
                continue;
            }

            int is_ok = 1;
            if(parametre[2] == 1){
                size_t len = strlen(entry->d_name);
                size_t elen = strlen(extension);
                if(!(len > elen && strcmp(entry->d_name + len - elen, extension) == 0 && entry->d_name[len - elen - 1] == '.')){
                    is_ok = 0;
                }
            }

            if(parametre[3] == 1){
                if(type[0] == 'f' && !S_ISREG(st.st_mode)) is_ok = 0;
                if(type[0] == 'd' && !S_ISDIR(st.st_mode)) is_ok = 0;
                if(type[0] == 'l' && !S_ISLNK(st.st_mode)) is_ok = 0;
                if(type[0] == 'p' && !S_ISFIFO(st.st_mode)) is_ok = 0;
            }

            if(is_ok == 1){
                for (int i = 0; indice_variable[i] != -1 ; i++) {
                    char *val = strdup(p);
                    if(val == NULL){
                        print("fsh: for: Erreur d'allocation\n", STDERR_FILENO);
                        free(p);
                        closedir(dir);
                        return last_status;
                    }
                    if(parametre[2] == 1){
                        size_t len = strlen(val);
                        size_t elen = strlen(extension);
                        if (len > elen + 1) {
                            val[len - elen - 1] = '\0';
                        }
                    }
                    if(strcmp(variable, commande[indice_variable[i]]) == 0){
                        free(commande[indice_variable[i]]);
                        commande[indice_variable[i]] = val;
                    }
                    else{
                        char *tmp = remplace_variable(cmd[indice_variable[i] + sauvegarde], val, variable);
                        free(commande[indice_variable[i]]);
                        free(val);
                        if(tmp == NULL){
                            free(p);
                            closedir(dir);
                            return last_status;
                        }
                        commande[indice_variable[i]] = tmp;
                    }
                }
                if(parametre[4]==1 && max_parallel > 1){
                    pid_t pid_enfant = fork();
                    if(pid_enfant == -1){
                        perror("fork");
                        continue;
                    }
                    if (pid_enfant == 0) {
                        int status = execute_all_commands(commande, last_status);
                        exit(status);
                    }
                    else{
                        pid_enfants[max_parallel - 1] = pid_enfant;
                        max_parallel= max_parallel - 1;
                    }
                }
                else{
                    int status = execute_all_commands(commande, last_status);
                    if (status == SIGINT + 128) {
                        free(p);
                        closedir(dir);
                        return status;
                    }
                    if(status > last_status) last_status = status;
                }
            }

            if(parametre[1] == 1 && S_ISDIR(st.st_mode)){
                last_status = parcours_recursif(p, cmd, commande, parametre, extension, type, variable, indice_variable, last_status, sauvegarde, depth + 1, max_parallel);
            }

            free(p);
        }
    }
    closedir(dir);
    for (int i = 0; i < parallel; i++) {
        int status;
        waitpid(pid_enfants[i], &status, 0);
        if(WIFEXITED(status)){
            status = WEXITSTATUS(status);
        }
        if(status > last_status) last_status = status;
    }
    return last_status;
}

int execute_for(char **cmd) {
    int last_status = 0;
    int parametre[5] = {0, 0, 0, 0, 0};
    char *extension = NULL;
    char *type = NULL;
    int max_parallel = 0;

    if (length(cmd) < 6) {
        print("fsh: for: Erreur de syntaxe\n", STDERR_FILENO);
        return 1;
    }

    char *variable = malloc(strlen(cmd[1]) + 2);
    if(variable == NULL){
        print("fsh: for: Erreur d'allocation\n", STDERR_FILENO);
        return 1;
    }
    strcpy(variable, "$");
    strcat(variable, cmd[1]);

    if(strcmp(cmd[2], "in") != 0){
        print("fsh: for: Erreur de syntaxe\n", STDERR_FILENO);
        free(variable);
        return 1;
    }

    char *directory = cmd[3];

    struct stat st;
    if(stat(directory, &st) == -1){
        char buffer[512];
        int len = snprintf(buffer, sizeof(buffer), "command_for_run: %s\ncommand_for_run: No such file or directory\n", directory);
        write(STDERR_FILENO, buffer, len);
        free(variable);
        return 1;
    }
    if(!S_ISDIR(st.st_mode)){
        char buffer[256];
        int len = snprintf(buffer, sizeof(buffer), "command_for_run: Not a directory\n");
        write(STDERR_FILENO, buffer, len);
        free(variable);
        return 1;
    }

    int pos = 4;
    while (cmd[pos] != NULL && strcmp(cmd[pos], "{") != 0) {
        if (strcmp(cmd[pos], "-A") == 0) {
            parametre[0] = 1;
        }
        else if (strcmp(cmd[pos], "-r") == 0) {
            parametre[1] = 1;
        }
        else if (strcmp(cmd[pos], "-e") == 0) {
            parametre[2] = 1;
            pos++;
            if(cmd[pos] == NULL){
                print("fsh: for: Erreur de syntaxe\n", STDERR_FILENO);
                free(variable);
                return 1;
            }
            extension = malloc(strlen(cmd[pos]) + 1);
            if(extension == NULL){
                print("fsh: for: Erreur d'allocation\n", STDERR_FILENO);
                free(variable);
                return 1;
            }
            strcpy(extension, cmd[pos]);
        }
        else if (strcmp(cmd[pos], "-t") == 0) {
            parametre[3] = 1;
            pos++;
            if(cmd[pos] == NULL){
                print("fsh: for: Erreur de syntaxe\n", STDERR_FILENO);
                free(variable);
                if(extension != NULL) free(extension);
                return 1;
            }
            type = malloc(strlen(cmd[pos]) + 1);
            if(type == NULL){
                print("fsh: for: Erreur d'allocation\n", STDERR_FILENO);
                free(variable);
                if(extension != NULL) free(extension);
                return 1;
            }
            strcpy(type, cmd[pos]);
        }
        else if (strcmp(cmd[pos], "-p") == 0) {
            parametre[4] = 1;
            pos++;
            if(cmd[pos] == NULL){
                print("fsh: for: Erreur de syntaxe\n", STDERR_FILENO);
                free(variable);
                if(extension != NULL) free(extension);
                if(type != NULL) free(type);
                return 1;
            }
            max_parallel = atoi(cmd[pos]);
        }
        else{
            print("fsh: for: paramètre inconnu\n", STDERR_FILENO);
            free(variable);
            if(extension != NULL) free(extension);
            if(type != NULL) free(type);
            return 1;
        }
        pos++;
    }

    if(cmd[pos] == NULL || strcmp(cmd[pos], "{") != 0 || strcmp(cmd[length(cmd)-1], "}") != 0){
        print("fsh: for: Erreur de syntaxe\n", STDERR_FILENO);
        free(variable);
        if(extension != NULL) free(extension);
        if(type != NULL) free(type);
        return 1;
    }

    pos += 1;

    int cmd_length = length(cmd);
    char **commande = malloc(sizeof(char*) * (cmd_length - 4));
    if(commande == NULL){
        print("fsh: for: Erreur d'allocation\n", STDERR_FILENO);
        free(variable);
        if(extension != NULL) free(extension);
        if(type != NULL) free(type);
        return 1;
    }
    int y = 0;
    int sauvegarde = pos;

    while(pos < cmd_length - 1){
        if(cmd[pos] == NULL){
            print("fsh: for: Erreur de syntaxe\n", STDERR_FILENO);
            for(int i = 0; i < y; i++) free(commande[i]);
            free(commande);
            free(variable);
            if(extension != NULL) free(extension);
            if(type != NULL) free(type);
            return 1;
        }
        commande[y] = malloc(strlen(cmd[pos]) + 1);
        if(commande[y] == NULL){
            print("fsh: for: Erreur d'allocation\n", STDERR_FILENO);
            for(int i = 0; i < y; i++) free(commande[i]);
            free(commande);
            free(variable);
            if(extension != NULL) free(extension);
            if(type != NULL) free(type);
            return 1;
        }
        strcpy(commande[y], cmd[pos]);
        pos++;
        y++;
    }
    commande[y] = NULL;

    int *indice_variable = pos_indice(commande, variable);
    if(indice_variable == NULL){
        for(int i = 0; commande[i] != NULL; i++){
            free(commande[i]);
        }
        free(commande);
        free(variable);
        if(extension != NULL) free(extension);
        if(type != NULL) free(type);
        return 1;
    }

    last_status = parcours_recursif(directory, cmd, commande, parametre, extension, type, variable, indice_variable, last_status, sauvegarde, 0, max_parallel);

    free(indice_variable);
    for(int i = 0; commande[i] != NULL; i++){
        free(commande[i]);
    }
    free(commande);
    free(variable);
    if(extension != NULL) free(extension);
    if(type != NULL) free(type);
    return last_status;
}
