#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#define PAGE 4096


int main() {
    int fd = open("test.txt", O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        exit(1);
    }

    printf("size = %lld\n", sb.st_size);

    char *file_in_memory = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_in_memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    for (int i = 0; i < sb.st_size; i++) {
        if (i % 2 == 0) {
            file_in_memory[i] = toupper(file_in_memory[i]);
        }
        printf("%c", file_in_memory[i]);
    }
    printf("\n");

    munmap(file_in_memory, sb.st_size);
    close(fd);
}
