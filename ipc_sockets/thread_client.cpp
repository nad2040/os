#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "requests.h"

#define BUFFSIZE 4096

int menu() {
    int choice = -1;
    while (choice < 0 || choice > 5) {
        printf("\n0. HELLO\n");
        printf("1. WIFI\n");
        printf("2. CALENDAR\n");
        printf("3. DATETIME\n");
        printf("4. FORTUNE\n");
        printf("5. QUIT\n");
        scanf("%d", &choice);
        if (choice < 0 || choice > 5) printf("Choose correctly!\n");
    }
    return choice;
}

int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "usage: ./thread_client <ip> <port>"); exit(1); }

    int fd = socket(AF_INET, SOCK_STREAM, 0); // use TCP IPv4
    if (fd < 0) perror("socket");

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2])); // port 8080
    inet_aton(argv[1], &(addr.sin_addr)); // set address to localhost

    int yes=1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int)) < 0) perror("setsockopt"); // setsockopt for no delay

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); exit(1); } // connect

    // MESSAGE Request
    char msg[BUFFSIZE], rsp[BUFFSIZE];
    Request req;

    const uint16_t magic = 1234;
    req.magic = magic;
    req.fd = fd;

    int n;

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    while (1) {
        req.reqtype = menu();
        printf("you chose %d\n\n", req.reqtype);
        if (req.reqtype == 5) {exit(1);}
        if (write(fd, (char*)&req, sizeof(req)) < 0) { perror("send fail"); exit(1); }
        
        while(1) {
            int rc = poll(fds, 1, 10000);
            if (fds[0].revents & POLLIN) {
                if ((n = read(fd, rsp, BUFFSIZE)) < 0) { perror("read"); exit(1); }
                else if (n == 0) { printf("connection to server dropped\n"); exit(1); }
                if (strcmp(rsp+n-5, "done")==0) {
                    memset((rsp+n-5),0,4);
                    write(STDOUT_FILENO, rsp, n);
                    break;
                }
                write(STDOUT_FILENO, rsp, n);
            } else break;
        }
    }
    // close(fd);
}
