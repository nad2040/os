#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main(void)
{

    if (lseek(STDIN_FILENO, 0, SEEK_CUR) == -1) {
        printf("%s %d\n", strerror(errno), errno);
        
        //perror("seek failure");
        
        printf("cannot seek\n");
    }
    else
        printf("seek OK\n");
    exit(0);
}