#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


char *render_static_file(char *filename) {
	FILE *file = fopen(filename, "r");

	if (file == NULL) {
		return NULL;
	} else {
		printf("%s does exist \n", filename);
	}

	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *temp = malloc(sizeof(char) * (fsize+1));
	char ch;
	int i = 0;
	while((ch = fgetc(file)) != EOF) {
		temp[i] = ch;
		i++;
	}
	fclose(file);
	return temp;
}

int main() {
    int server_socket;
    int port=8000;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror("socket"); // use TCP && IPv4
    printf("Server FD: %d\n", server_socket);

    struct sockaddr_in server_address = { 0 };
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // allow all connections and make sure to TURN OFF FIREWALL!!!
    server_address.sin_port = htons(port);

    int yes = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) perror("setsockopt"); // setsockopt to reuse port
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) perror("bind"); // bind
    if (listen(server_socket, 4) < 0) perror("listen"); // listen

    while (1) { // accept loop
        char client_msg[4096] = "";
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) perror("accept");
        
        // printf("Client tried to connect\n");

        read(client_socket, client_msg, 4095);
		// printf("%s\n", client_msg);
        char *request = strtok(client_msg, "\r\n");
        printf("%s\n",request);

        char *response_data = render_static_file("test.html");

        char http_header[4096] = "HTTP/1.1 200 OK\r\n\r\n";

		strcat(http_header, response_data);
		strcat(http_header, "\r\n\r\n");

		send(client_socket, http_header, sizeof(http_header), 0);
        close(client_socket);
		free(response_data);

        sleep(1);
    }
}