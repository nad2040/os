#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFSIZE 1
int main(void)
{
    int n;
    char buf[BUFFSIZE];
    while ((n = read(0, buf, BUFFSIZE)) > 0)
        if (write(1, buf, n) != n)
            { fprintf(stderr, "write error"); exit(1); }
    if (n < 0)
        { fprintf(stderr, "read error"); exit(1); }
    exit(0);
}