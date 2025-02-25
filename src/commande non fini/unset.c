#include <stdlib.h>
#include <stdio.h>

void unset_command(const char *name) {
    if (unsetenv(name) == -1) {
        perror("unset");
    }
}
