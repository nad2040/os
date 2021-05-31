#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "apue.h"

int main(void)
{
    int c;
    while ((c = getc(stdin)) != EOF){
        if (putc(c, stdout) == EOF)
            fprintf(stderr, "output error");
    }
    if (ferror(stdin))
        fprintf(stderr, "input error");
    
    exit(0); 
}

