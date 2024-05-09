
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "stdio.h"
#include "arpa/inet.h"
#include "fwrapper.h"

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

int Connect(int fd, struct sockaddr_in *adr, socklen_t socklen) {
    int res = connect(fd, (struct sockaddr *) adr, socklen);
    if (res == -1) {
        perror("Error to connect\n");
        return -1;
    }
    return 1;
}

int Inet_pton(int af, const char *src, void *dst) {
    int res = inet_pton(af, src, dst);
    if (res == 0) {
        return -1;
    }
    if (res == -1) {
        return -1;
    } else {
        return 11;
    }
}
