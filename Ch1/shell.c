#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static void sig_int(int);
/* our signal-catching function */
/* from apue.h */
#define MAXLINE 4096

int main(void) {
    char buf[MAXLINE];
    pid_t pid;
    int status;

    if (signal(SIGINT, sig_int) == SIG_ERR) {printf("signal error"); exit(1);}

    printf("%% "); /* print prompt (printf requires %% to print %) */
    while (fgets(buf, MAXLINE, stdin) != NULL)
    {
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0; /* replace newline with null */
        if ((pid = fork()) < 0)
        {
            printf("fork error"); exit(1);
        }
        else if (pid == 0)
        { /* child */
            execlp(buf, buf, (char *)0);
            printf("couldn’t execute: %s", buf); 
            exit(127);
        }
        /* parent */
        if ((pid = waitpid(pid, &status, 0)) < 0) { printf("waitpid error"); exit(1); }
        printf("%% ");
    }
    exit(0);
}

void sig_int(int signo)
{
    printf("interrupt\n%% ");
}