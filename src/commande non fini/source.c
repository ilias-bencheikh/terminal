#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void execute_line(const char *line);

void source_command(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("source");
        return;
    }

    char buffer[1024];
    ssize_t n;
    char *line = NULL;
    size_t len = 0;

    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        size_t i = 0;
        while (i < n) {
            if (buffer[i] == '\n') {
                if (line) {
                    execute_line(line);
                    free(line);
                    line = NULL;
                    len = 0;
                }
                i++;
            } else {
                size_t start = i;
                while (i < n && buffer[i] != '\n') {
                    i++;
                }
                size_t chunk_len = i - start;
                line = realloc(line, len + chunk_len + 1);
                memcpy(line + len, buffer + start, chunk_len);
                len += chunk_len;
                line[len] = '\0';
            }
        }
    }

    if (line) {
        execute_line(line);
        free(line);
    }

    if (n == -1) {
        perror("read");
    }
    close(fd);
}
