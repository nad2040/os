#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *env_init[] = {"USER=unknown", "PATH=/tmp", NULL};

int main(void)
{
    // pid_t pid;
    // if ((pid = fork()) < 0) { fprintf(stderr, "fork error"); exit(1); }
    // else if (pid == 0)
     /* specify pathname, specify environment */
        if (execle("../Ch7/echoall", "echoall", "myarg1", "MY ARG2", (char *)0, env_init) < 0) { fprintf(stderr, "execle error"); exit(1); }
        printf("hello");
        if (execlp("../Ch7/echoall", "echoall", "only 1 arg", (char *)0) < 0) { fprintf(stderr, "execlp error"); exit(1); }
    
    exit(0);
}