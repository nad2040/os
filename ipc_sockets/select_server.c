#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFSIZE 4096
int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // use TCP && IPv4
    if (server_fd < 0) perror("socket");
    printf("ServerFD: %d\n", server_fd);

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080); // port 8080

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) perror("setsockopt"); // setsockopt to reuse port
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) perror("bind"); // bind
    if (listen(server_fd, 4) < 0) perror("listen"); // listen
    
    int nfd = server_fd;
    fd_set fds; // all read bits
    FD_ZERO(&fds); // setup/clear all bits to 0
    FD_SET(server_fd, &fds);

    while (1) { // select loop
        fd_set readfds = fds; // take copy of all read bits every cycle to serve all sockets

        int ready = select(nfd+1, &readfds, NULL, NULL, NULL); // select for read-ready fds
        if (ready < 0) perror("select");
        else if (ready == 0) { printf("timeout\n"); continue; }
        else {
            for (int fd=3; fd<nfd+1; ++fd) { // ignore 0, 1, and 2
                if (FD_ISSET(fd, &readfds)) { // check if fd is ready to read
                    // printf("fd: %d is set\n", fd);
                    if (fd == server_fd) { // if fd is server, check for new connections
                        int sock = accept(server_fd, NULL, NULL); // accept new socket
                        if (sock < 0) perror("accept");
                        printf("Accepted socket fd: %d\n", sock);

                        if (sock > nfd) nfd = sock; // update nfd
                        FD_SET(sock, &fds); // add sock to readfds
                    }
                    else { // if fd is client socket, serve client
                        char buf[BUFFSIZE];
                        int len;
                        if ((len = read(fd, buf, sizeof(buf))) < 0) perror("read");
                        else if (len == 0) { printf("%d dropped\n", fd); FD_CLR(fd, &fds); }
                        else {
                            if (write(fd, buf, len) != len) perror("write");
                            printf("Served fd: %d\n", fd);
                        }
                    }
                }
            }
        }
    }
}