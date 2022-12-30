#pragma once
#include <unistd.h>
#include <iostream>
#include <thread>
#include <queue>
#include <cstring>
#include "requests.h"

class Worker {
public:
    Worker() {
        wthread = std::thread([this](){doWork();});
    }
    ~Worker() {
        wthread.join();
    }

    void addWork(const Request& req) {
        wq.push(req);
    }
    void doWork() {
        while (1) {
            if (!wq.empty()) {
                Request req = wq.front(); wq.pop();
                handle_event(req);
            }
            usleep(100000);
        }
    }

private:
    void handle_event(const Request& req) {
        std::cout << "Request:  Key: " << req.magic << ", Type: " << req.reqtype << ", FD: " << req.fd << ", Thread: " << std::this_thread::get_id() << std::endl;

        switch (req.reqtype) {
            case HELLO: processRequest("cowsay hello", req.fd); break;
            case WIFI: processRequest("airport -s", req.fd); break;
            case CALENDAR: processRequest("cal -3", req.fd); break;
            case DATE: processRequest("date", req.fd); break;
            case FORTUNE: processRequest("fortune", req.fd); break;
        }
    }

    void processRequest(const char *command, int fd) {
        FILE* fs = popen(command, "r");
        char buf[4096] = {0};
        int *len = (int*)buf;
        int sum=0;
        while (fgets(buf+sizeof(int), sizeof(buf)-sizeof(int), fs) != NULL) {
            *len = strlen(buf+4);
            int n = write(fd, buf, sizeof(int)+(*len));
            sum += n;
        }
        *len = 0;
        write(fd, buf, sizeof(int));
        sum += sizeof(int);
        printf("Response: %d bytes\n", sum);
    }
    std::queue<Request> wq;
    std::thread wthread;
};
