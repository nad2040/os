#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;
    if ((pid = fork()) < 0) { fprintf(stderr, "fork error"); exit(1); }
    else if (pid == 0)
    {
        if (execl("/Users/danliu/Desktop/work/os/Ch8/testinterp", "testinterp", "myarg1", "MY ARG2", (char *)0) < 0)
            { fprintf(stderr, "execl error"); exit(1); }
    }
    if (waitpid(pid, NULL, 0) < 0) { fprintf(stderr, "waitpid error"); exit(1); }
    exit(0);
}