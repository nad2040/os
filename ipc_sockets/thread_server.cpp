#include <unistd.h>
#include <stdio.h>
#include <thread>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "worker.h"

#define BUFFSIZE 4096
int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // use TCP && IPv4
    if (server_fd < 0) perror("socket");
    printf("ServerFD: %d\n", server_fd);

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080); // port 8080

    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) perror("setsockopt"); // setsockopt to reuse port
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) perror("bind"); // bind
    if (listen(server_fd, 4) < 0) perror("listen"); // listen

    // POLL

    #define MAX_CONN 15  // num of clients, so +1 to include server_fd
    struct pollfd fds[MAX_CONN+1] = {0};
    int timeout = 10000;
    
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    int nfd = 1;

    // WORKER

    Worker workers[2] = { Worker(), Worker() };

    while (1) { // poll loop
        int ready = poll(fds, nfd, timeout); // poll for ready fds
        if (ready < 0) { perror("poll"); exit(1); }
        else if (ready == 0) { 
            //printf("timeout\n");
            continue;
        }
        // fd is server, check for new connections
        if (fds[0].revents & POLLIN) {
            int sock = accept(server_fd, NULL, NULL); // accept new socket
            if (sock < 0) perror("accept");

            if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int)) < 0) perror("setsockopt"); // setsockopt for no delay
            
            if (nfd < MAX_CONN+1) { // check connections.
                fds[nfd].fd = sock;
                fds[nfd].events = POLLIN;
                ++nfd;
                printf("Accepted socket fd: %d\n", sock);
            } else { // max connections. disconnect
                write(sock, "Too many connections", 21);
                if (close(sock) < 0) perror("close");
                printf("Denied socket connection: %d\n", sock);
            }
        }

        for (int cur_pfd = 1; cur_pfd < nfd; ++cur_pfd) {
            if (fds[cur_pfd].fd == 0 || fds[cur_pfd].revents == 0) continue;
            printf("pfd:%d fd:%x events:%x revents:%x \n", cur_pfd, fds[cur_pfd].fd, fds[cur_pfd].events, fds[cur_pfd].revents);
            
            // fd is client, serve client
            if (fds[cur_pfd].revents & POLLIN) {
                char buf[BUFFSIZE];
                int len;
                if ((len = read(fds[cur_pfd].fd, buf, sizeof(buf))) < 0) perror("read");
                else if (len == 0) {
                    printf("%d dropped\n", fds[cur_pfd].fd);
                    if (close(fds[cur_pfd].fd) < 0) perror("close");
                    fds[cur_pfd].fd = 0;
                    fds[cur_pfd].events = 0;
                    fds[cur_pfd].revents = 0;
                    continue;
                }
                else {
                    Request *req = (Request*)buf;
                    req->fd = fds[cur_pfd].fd;
                    workers[req->fd % 2].addWork(*req);
                }
            }
        }
    }
}
