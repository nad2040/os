#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "HashTable.h"

typedef struct HTTP_Server {
    int socket;
    int port;
} HTTP_Server;

HTTP_Server *make_HTTP_Server(int port) {
    HTTP_Server *http_server = malloc(sizeof(HTTP_Server));
    int server;
    if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror("socket"); // use TCP && IPv4
    printf("server fd: %d\n", server);

    struct sockaddr_in server_address = { 0 };
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // allow all connections and make sure to TURN OFF FIREWALL!!!
    server_address.sin_port = htons(port);

    int yes = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) perror("setsockopt"); // setsockopt to reuse port
    if (bind(server, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) perror("bind"); // bind
    if (listen(server, 4) < 0) perror("listen"); // listen

    http_server->socket = server;
    http_server->port = port;
    return http_server;
}
void delete_HTTP_Server(HTTP_Server *http_server) {
    close(http_server->socket);
    free(http_server);
}

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

int hash_str(void *key) {
    char *str = (char *)key;
    int hash = 0;
    for (int i = 0; i < strlen(str); i++) {
        hash += str[i];
    }
    return hash;
}
int cmp_str(void *key1, void *key2) {
    char *str1 = (char *)key1;
    char *str2 = (char *)key2;
    return strcmp(str1, str2);
}

int main() {
    const char *HEADER_HTTP_OK = "HTTP/1.1 200 OK\r\n\r\n";
    const char *HEADER_HTTP_NOT_FOUND = "HTTP/1.1 404 Not Found\r\n\r\n";
    HTTP_Server *web_server = make_HTTP_Server(8000);

    HashTable *routes = make_hash_table(20, &hash_str, &cmp_str, NULL);
    hash_table_set(routes, "/", "index.html");
    hash_table_set(routes, "/a", "a.html");
    hash_table_set(routes, "/aboutme", "aboutme.html");
    hash_table_set(routes, "/b", "b.html");

    while (1) { // accept loop
        char client_msg[4096] = "";
        int client_socket = accept(web_server->socket, NULL, NULL);
        if (client_socket < 0) perror("accept");

        read(client_socket, client_msg, 4095);

        char *request_http_header = strtok(client_msg, "\r\n");
        // printf("request is: %s\n",request_http_header);

        char *request_type = strtok(request_http_header, " ");
        char *request_path = strtok(NULL, " ");
        char *request_protocol = strtok(NULL, " ");

        char *filename = hash_table_get(routes, request_path);
        char *file;
        char response[4096] = "";

        if (filename != NULL) {
            printf("filename is \"%s\"\n", filename);
            file = render_static_file(filename);
            strcpy(response, HEADER_HTTP_OK);
            strcat(response, file);
            free(file);
        } else {
            printf("filename is NULL\n");
            strcpy(response, HEADER_HTTP_NOT_FOUND);
        }

		strcat(response, "\r\n\r\n");

		send(client_socket, response, sizeof(response), 0);
        close(client_socket);
        sleep(1);
    }

    delete_hash_table(routes);
    delete_HTTP_Server(web_server);
}
