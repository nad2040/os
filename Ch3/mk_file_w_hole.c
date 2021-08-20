#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

char buf1[] = "abcdefghij";
char buf2[] = "ABCDEFGHIJ";
int main(void)
{
    int fd;
    if ((fd = creat("file.hole", FILE_MODE)) < 0)
        { fprintf(stderr, "creat error"); exit(1); }
    if (write(fd, buf1, 10) != 10)
        { fprintf(stderr, "buf1 write error"); exit(1); }
    /* offset now = 10 */
    if (lseek(fd, 16384, SEEK_SET) == -1)
        { fprintf(stderr, "lseek error"); exit(1); }
    /* offset now = 16384 */
    if (write(fd, buf2, 10) != 10)
        { fprintf(stderr, "buf2 write error"); exit(1); }
    /* offset now = 16394 */
    exit(0);
}