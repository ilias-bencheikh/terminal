#include <unistd.h>
#include <string.h>

extern char **environ;
void print(char* string , int sortie);

void set_command() {
    for (char **env = environ; *env != 0; env++) {
        print(*env, STDOUT_FILENO);
        print("\n", STDOUT_FILENO);
    }
}
