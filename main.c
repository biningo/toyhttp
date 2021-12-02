#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int get_server(int port) {
    int server = socket(PF_INET, SOCK_STREAM, 0);
    if (server == -1) {
        exit(-1);
    }
    struct sockaddr_in name;
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server, (struct sockaddr *) &name, sizeof(name)) < 0) {
        exit(-2);
    }
    if (listen(server, 5) < 0) {
        exit(-3);
    }
    return server;
}


int wait_client(int server) {
    struct sockaddr_in name;
    unsigned name_len = sizeof(name);
    int client = accept(server, (struct sockaddr *) &name, &name_len);
    if (client < 0) {
        exit(-4);
    }
    return client;
}

void read_line(int client, char *buf) {
    int count = 0;
    char c = 0;
    ssize_t n;
    while (c != '\n') {
        n = recv(client, &c, (size_t) 1, 0);
        if (n < 0) {
            c = '\n';
        }
        buf[count++] = c;
    }
    buf[count] = '\0';
}

void response_ok(int client) {
    char buf[1024];
    char *data = "<h1>Hello,World</h1>";

    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Server: myserver \r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Length: %d\r\n", (int) strlen(data));
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "%s\n", data);
    send(client, buf, strlen(buf), 0);
}

void handler(int client) {
    char buf[1024];
    read_line(client, buf);
    while (strcmp("\r\n", buf) != 0) {
        printf("%s", buf);
        read_line(client, buf);
    }
    printf("-----------------------------\n");
    response_ok(client);
    close(client);
}


int main() {
    int server = get_server(8000);
    while (1) {
        int client = wait_client(server);
        handler(client);
    }
}


