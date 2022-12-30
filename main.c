#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>

void show_usage(char *);
void file_stat(char *, struct stat *);

int main(int argc, char *argv[])
{
    char *path = NULL;
    struct stat st;
    if (argc < 2)
    {
        show_usage(argv[0]);
    }
    path = argv[1];
    file_stat(path, &st);

    printf("permissions: %d\ninode: %lld\ndevice: %d\nuid: %d\ngid: %d\nlast modified: %ld\nlast accessed: %ld\nlinks: %lld\nsize: %dB\n",
            st.st_mode, st.st_ino, st.st_dev, st.st_uid, st.st_gid, st.st_atime, st.st_mtime, st.st_size);
    return EXIT_SUCCESS;
}

void show_usage(char *progname) 
{
    printf("Usage: %s <file>\n", progname);
}

void file_stat(char *path, struct stat *st)
{
    int fd = open(path, O_RDONLY);

    if (fd < 0) 
    {
        perror("open()");
        exit(EXIT_FAILURE);
    }
    if (fstat(fd, st) < 0) 
    {
        perror("fstat()");
        exit(EXIT_FAILURE);
    }
}
