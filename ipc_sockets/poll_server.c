#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
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

    // POLL

    #define MAX_CONN 5  // num of clients, so +1 to include server_fd
    struct pollfd fds[MAX_CONN+1] = {0};
    int timeout = 1000;
    
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    int nfd = 1;

    while (1) { // poll loop
        int ready = poll(fds, nfd, timeout); // poll for ready fds
        if (ready < 0) { perror("poll"); exit(1); }
        else if (ready == 0) { printf("timeout\n"); continue; }

        // fd is server, check for new connections
        if (fds[0].revents & POLLIN) {
            int sock = accept(server_fd, NULL, NULL); // accept new socket
            if (sock < 0) perror("accept");
            
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
            if (fds[cur_pfd].fd < 0) continue;
            if ((fds[cur_pfd].revents & POLLHUP)) {
                printf("%d dropped\n", fds[cur_pfd].fd);
                if (close(fds[cur_pfd].fd) < 0) perror("close");
                fds[cur_pfd].fd = -1;
                continue; // return to poll loop if any connection drops
            }
            if (fds[cur_pfd].revents == 0) continue;

            printf("pfd:%d fd:%x events:%x revents:%x \n", cur_pfd, fds[cur_pfd].fd, fds[cur_pfd].events, fds[cur_pfd].revents);

             // fd is client, serve client
            char buf[BUFFSIZE];
            int len;
            if ((len = read(fds[cur_pfd].fd, buf, sizeof(buf))) < 0) perror("read");
            else {
                if (write(fds[cur_pfd].fd, buf, len) != len) perror("write");
                printf("Served fd: %d\n", fds[cur_pfd].fd);
            }
        }
    }
}