#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    pid_t pid;
    char command[256];
} Job;

extern Job jobs[];
extern int job_count;
void print(char* string , int sortie);

void execute_bg(int job_id) {
    if (job_id > 0 && job_id <= job_count) {
        pid_t pid = jobs[job_id - 1].pid;
        if (kill(pid, SIGCONT) == -1) {
            perror("bg");
        }
    } else {
        print("bg: job inexistant\n", STDERR_FILENO);
    }
}
