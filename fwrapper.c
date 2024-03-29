//
// Created by vadim on 28.3.24.
//
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include "stdio.h"
#include "arpa/inet.h"
#include "fwrapper.h"

struct packet {
    char buffer[1024];
    char stop[3];
};

char *createArrayFromStruct(struct packet pcg) {
    char *array = malloc(1027);
    for (int i = 0; i < 1024; i++) {
        array[i] = pcg.buffer[i];
    }
    for (int i = 1024; i < 1027; i++) {
        array[i] = pcg.stop[i - 1024];
    }
    return array;
}

typedef struct {
    char buffer[1024];
    char stop[3];
} packet;

void send_file(const char *path, int fd) {
    FILE *file = fopen(path, "rb");
    printf("file %s is open\n", path);
    char buffer[1027] = {'+'};
    int total = 0;
    size_t n = 0;
    while (1) {
        n = fread(buffer, 1, 1024, file);
        if(n != 1024){
            buffer[n] = '-';
            buffer[n+1] = '-';
            buffer[n+2] = '-';
            send(fd,buffer,1027,0);
            break;
        }
        send(fd,buffer,1027,0);
    }
    fclose(file);
}

void recv_file(const char *path, int fd) {
    FILE *file = fopen(path, "wb");
    printf("file %s is open\n", path);
    char buffer[1027];
    int total = 0;
    size_t n = 0;
    while (1) {
        n = recv(fd, buffer, 1027, 0);
        if(buffer[n-1] == '-' && buffer[n-2] == '-' && buffer[n-3] == '-'){
            fwrite(buffer,1,n-3,file);
            break;
        }
        fwrite(buffer,1,n-3,file);
    }
    fclose(file);
}

int Socket(int domain, int type, int protocol) {
    int server = socket(domain, type, protocol);
    if (server == -1) {
        perror("fail on create socket\n");
        exit(EXIT_FAILURE);
    }
    return server;
}

void Bind(int socket, struct sockaddr_in *adr, socklen_t socklen) {
    int res = bind(socket, (struct sockaddr *) adr, socklen);
    if (res == -1) {
        perror("Error on bind socket\n");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfg, int backlog) {
    int res = listen(sockfg, backlog);
    if (res == -1) {
        perror("Listen failed\n");
        exit(EXIT_FAILURE);
    }
}

int Accept(int server, struct sockaddr_in *adr, socklen_t socklen) {
    int res = accept(server, (struct sockaddr *) adr, &socklen);
    if (res == -1) {
        perror("Error accept\n");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Connect(int fd, struct sockaddr_in *adr, socklen_t socklen) {
    int res = connect(fd, (struct sockaddr *) adr, socklen);
    if (res == -1) {
        perror("Error to connect\n");
        exit(EXIT_FAILURE);
    }
}

void Inet_pton(int af, const char *src, void *dst) {
    int res = inet_pton(af, src, dst);
    if (res == 0) {
        printf("fail on inet_pton\n");
    }
    if (res == -1) {
        printf("is valid ip\n");
    } else {
        perror("inet_pton");
    }
}