#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(void)
{
    int c;
    FILE* fileHandle = fopen("/tmp/output.txt", "w+");
    if (0 == fileHandle)
    {
        perror("failed to open file");
        exit(1);
    }
    while ((c = getc(stdin)) != EOF){
        //if (putc(c, stdout) == EOF)
        if (fputc(c, fileHandle) == EOF) {
            fprintf(stderr, "output error");
        }
        fflush(fileHandle);
    }
    if (ferror(stdin))
        fprintf(stderr, "input error");
    
    exit(0); 
}

