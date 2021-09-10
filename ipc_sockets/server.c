#include <sys/socket.h>
#include <sys/types.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFSIZE 4096
int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // use TCP IPv4
    if (server_fd < 0) perror("socket");

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080); // port 8080
    //inet_aton("127.0.0.1", &(addr.sin_addr)); // set address to localhost

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) perror("bind"); // bind
    if (listen(server_fd, 4) < 0) perror("listen"); // listen

    while (1) { // accept loop
        int sock = accept(server_fd, NULL, NULL);
        if (sock < 0) perror("accept");
        
        char buf[BUFFSIZE];
        int len;
        if ((len = read(sock, buf, sizeof(buf))) < 0) perror("read");
        if (write(sock, buf, len) != len) perror("write error");
        sleep(1);
    }
}