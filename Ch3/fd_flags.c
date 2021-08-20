#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

void printFdFlags(int fd) 
{
    int val;
    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        { fprintf(stderr, "fcntl error for fd %d", fd); exit(1)  ; }
    switch (val & O_ACCMODE)
    {
    case O_RDONLY:
        printf("read only");
        break;
    case O_WRONLY:
        printf("write only");
        break;
    case O_RDWR:
        printf("read write");
        break;
    default:
        { printf("unknown access mode"); exit(1); }
    }
    if (val & O_APPEND)
        printf(", append");
    if (val & O_NONBLOCK)
        printf(", nonblocking");
    if (val & O_SYNC)
        printf(", synchronous writes");
#if !defined(_POSIX_C_SOURCE) && defined(O_FSYNC) && (O_FSYNC != O_SYNC)
    if (val & O_FSYNC)
        printf(", synchronous writes");
#endif
    putchar('\n');
}
int main(int argc, char *argv[])
{
    
    // if (argc != 2)
    //     { fprintf(stderr, "usage: a.out <descriptor#>"); exit(1); }
    
    int fd1 = open("/tmp/tmp.out", O_RDONLY | O_CREAT);
    if (fd1 < 0) {
        perror("failed to open file");
        exit(1);
    }
    //printFdFlags(atoi(argv[1]));
    printFdFlags(fd1);

    exit(0);
}