#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "ch3.h"

#include <time.h>

class timing {
public:
    timing() {
        if (clock_gettime(CLOCK_REALTIME, &tstart_) != 0) {
            perror("fail to get time");
            exit(1);
        }
    }

    ~timing() {
        if (clock_gettime(CLOCK_REALTIME, &tstop_) != 0) {
            perror("fail to get time");
            exit(1);
        }
        std::cerr << "time taken:" << diff() << "ns\n";
    }

    uint64_t diff() {
        return (tstop_.tv_sec - tstart_.tv_sec) * 1000000000 + (tstop_.tv_nsec - tstart_.tv_nsec);
    }
private:
    struct timespec tstart_;
    struct timespec tstop_;
};

#define BUFFSIZE 4096
int main(void)
{
    int n;
    char buf[BUFFSIZE];

    {
        timing ti;
        int outFd = open("tmp.img", O_RDWR | O_CREAT, 0600);
        if (outFd < 0) {
            perror("failed to open output file");
            exit(1);
        }
    // set_fl(STDOUT_FILENO, O_SYNC);
    while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0)
        if (write(outFd, buf, n) != n)
            { fprintf(stderr, "write error"); exit(1); }
    if (n < 0)
        { fprintf(stderr, "read error"); exit(1); }
    }
    exit(0);
}