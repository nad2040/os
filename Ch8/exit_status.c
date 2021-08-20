#include <sys/wait.h>
#include "ch8.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;
    int status;
    if ((pid = fork()) < 0)
        { fprintf(stderr, "fork error"); exit(1); }
    else if (pid == 0)
        exit(7);
    if (wait(&status) != pid)
        { fprintf(stderr, "wait error"); exit(1); }
    pr_exit(status);
    if ((pid = fork()) < 0)
        { fprintf(stderr, "fork error"); exit(1); }
    else if (pid == 0)
        abort();
    if (wait(&status) != pid)
        { fprintf(stderr, "wait error"); exit(1); }
    pr_exit(status);
    if ((pid = fork()) < 0)
        { fprintf(stderr, "fork error"); exit(1); }
    else if (pid == 0)
        status /= 0;
    if (wait(&status) != pid)
        { fprintf(stderr, "wait error"); exit(1); }
    pr_exit(status);

    exit(0);
}