#include <stdio.h>
#include <unistd.h>

/**
 * Cette fonction envoie une séquence d'échappement ANSI pour effacer l'écran du terminal.
 *
 * @param args rien
 */
void execute_clear() {
    write(STDOUT_FILENO, "\033[2J\033[H", 7);
    fflush(stdout);
}
