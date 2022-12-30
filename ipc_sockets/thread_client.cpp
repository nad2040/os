#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "requests.h"
#include <cassert>

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
    char rsp[BUFFSIZE];
    Request req;

    const uint16_t magic = 1234;
    req.magic = magic;
    req.fd = fd;

    // POLL
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    // LOOP
    while (1) {
        // SEND
        req.reqtype = menu();
        printf("you chose %d\n\n", req.reqtype);
        if (req.reqtype == 5) {exit(1);}
        if (write(fd, (char*)&req, sizeof(req)) < 0) { perror("send fail"); exit(1); }
        
        // RECEIVE
        while(1) {
            int rc = poll(fds, 1, 10000);
            if (fds[0].revents & POLLIN) {
                bool endReached = false; // flag for receiving ending length 0 header
                int remaining = 0;
                int n;
                while ((n = read(fd, rsp+remaining, BUFFSIZE-remaining)) > 0) { // read as much work into buffer
                    char *curpos = rsp; // every read, move pointer to beginning to start consumption
                    remaining += n; // count the bytes received

                    // printf("received %d bytes\n", n);
                    // printf("remaining: %d bytes\n", remaining);

                    while (remaining >= 4 && remaining >= *(int*)curpos + sizeof(int)) { // there is full message left in the buffer
                        int len = *(int*)curpos; // read the length header
                        if (len == 0) { // received ending length 0 header. TODO: increase complexity with a better header.
                            endReached = true; break;
                        }
                        // printf("len is %d\n", len);

                        write(STDOUT_FILENO, curpos+4, len); // write message

                        curpos += sizeof(int) + len; // increment offset
                        remaining -= sizeof(int) + len; // decrement remaining bytes
                    }
                    if (endReached) break;
                    memmove(rsp, curpos, remaining); // move remaining cutoff bytes to the beginning and go read again for more work to consume
                }
                if (n == 0) { printf("connection to server dropped\n"); exit(1); } // error checking on read
                else if (n < 0) { perror("read"); exit(1); } // error checking on read
                break; // reading done
            }
        }
    }
}