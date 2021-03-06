#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/syslimits.h>

unsigned long long count;
struct timeval end;
void checktime(char *str) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (tv.tv_sec >= end.tv_sec && tv.tv_usec >= end.tv_usec)
    {
        printf("%s count = %lld\n", str, count);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    pid_t pid;
    char *s;
    int nzero, ret;
    int adj = 0;
    setbuf(stdout, NULL);
    nzero = NZERO;
    printf("NZERO = %d\n", nzero);
    if (argc == 2)
        adj = strtol(argv[1], NULL, 10);
    gettimeofday(&end, NULL);
    end.tv_sec += 10; /* run for 10 seconds */
    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "fork failed"); exit(1);
    }
    else if (pid == 0)
    { /* child */
        s = "child";
        printf("current nice value in child is %d, adjusting by %d\n",
               nice(0) + nzero, adj);
        errno = 0;
        if ((ret = nice(adj)) == -1 && errno != 0)
            {fprintf(stderr, "child set scheduling priority"); exit(1);}
        printf("now child nice value is %d\n", ret + nzero);
    }
    else
    { /* parent */
        s = "parent";
        printf("current nice value in parent is %d\n", nice(0) + nzero);
    }
    for (;;)
    {
        if (++count == 0) { fprintf(stderr, "%s counter wrap", s); exit(1); }
        checktime(s);
    }
}