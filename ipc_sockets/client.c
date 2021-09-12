#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "requests.h"

#define BUFFSIZE 4096
int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "usage: <ip> <port>"); exit(1); }

    int fd = socket(AF_INET, SOCK_STREAM, 0); // use TCP IPv4
    if (fd < 0) perror("socket");

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2])); // port 8080
    inet_aton(argv[1], &(addr.sin_addr)); // set address to localhost

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); exit(1); } // connect

    char msg[BUFFSIZE], rsp[BUFFSIZE];
    while (1) {
        int n = read(STDIN_FILENO, msg, BUFFSIZE);
        if (write(fd, msg, n) < 0) { perror("send fail"); exit(1); }

        if ((n = read(fd, rsp, BUFFSIZE)) < 0) { perror("recv fail"); exit(1); }
        else if (n == 0) { printf("connection to server dropped\n"); exit(1); }
        else write(STDOUT_FILENO, rsp, n);
    }


    /* CLIENT
       connect()
       read/write
     */
    /* SERVER
       bind
       listen
       accept
       read/write
     */
}