#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

#define MAXLINE 4096


static void sig_alrm(int);
static jmp_buf env_alrm;
int main(void)
{
    int n;
    char line[MAXLINE];
    if (signal(SIGALRM, sig_alrm) == SIG_ERR)
        {fprintf(stderr, "signal(SIGALRM) error"); exit(1);}
    if (setjmp(env_alrm) != 0)
        {printf("read timeout"); exit(1);}
    alarm(10);
    if ((n = read(STDIN_FILENO, line, MAXLINE)) < 0)
        {fprintf(stderr, "read error"); exit(1);}
    alarm(0);
    write(STDOUT_FILENO, line, n);
    exit(0);
}
static void
sig_alrm(int signo)
{
    longjmp(env_alrm, 1);
}