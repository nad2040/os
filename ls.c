//#include "apue.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/errno.h>

int main(int argc, char *argv[])
{
    DIR             *dp;
    struct dirent   *dirp;
    if (argc != 2) {
        printf("usage: ls directory_name");
        exit(1);
    }
    if ((dp = opendir(argv[1])) == NULL) {
        //print errno
        perror("can't open dir");
        /*
        char* errmsg = strerror(errno);
        printf("can’t open %s, errno:%s", argv[1], errmsg);
        */
        exit(1);
    }
    while ((dirp = readdir(dp)) != NULL)
        printf("%s\n", dirp->d_name);
    closedir(dp);
    exit(0);
}
