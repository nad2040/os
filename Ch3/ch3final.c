#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

int main(void) {
    // int fd1 = open("final.t", O_RDWR | O_CREAT, 0600);
    // if (fd1 < 0) { perror("failed to open output file"); exit(1); }

    printf("%d\n", NAME_MAX);


    exit(0);
}