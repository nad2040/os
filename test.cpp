#define _GNU_SOURCE
#include <thread>
#include <iostream>

#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

void getTime() {
    std::cout << "time" << '\n';
}

int main() {
    std::thread t[4];

    //may return 0 when not able to detect
    const auto processor_count = std::thread::hardware_concurrency();
    std::cout << processor_count << '\n';

    cpu_set_t cpuset;

    for (int i=0; i<4; ++i) {
        t[i] = std::thread(getTime);
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        int s = pthread_setaffinity_np(t[i], sizeof(cpuset), &cpuset);
        if (s != 0) perror("setaffinity");
    }

    for (int i=0; i<4; ++i) {
        t[i].join();
    }
}

