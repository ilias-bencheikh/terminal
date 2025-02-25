#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>

void read_command(const char *prompt, char *variable) {
    char *input = readline(prompt);
    if (input) {
        // Stocke la valeur dans une variable d'environnement ou interne
        setenv(variable, input, 1);
        free(input);
    }
}
