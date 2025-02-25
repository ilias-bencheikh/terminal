#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

typedef struct {
    pid_t pid;
    char command[256];
} Job;

#define MAX_JOBS 100

void print(char* string , int sortie);
Job jobs[MAX_JOBS];
int job_count = 0;

void add_job(pid_t pid, const char *command) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, command, sizeof(jobs[job_count].command) - 1);
        job_count++;
    }
}

void execute_jobs() {
    char buffer[512];
    for (int i = 0; i < job_count; i++) {
        int len = snprintf(buffer, sizeof(buffer), "[%d] %d %s\n", i + 1, jobs[i].pid, jobs[i].command);
        print(buffer, STDOUT_FILENO);
    }
}
