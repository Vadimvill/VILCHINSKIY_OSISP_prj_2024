#include "fwrapper.h"
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

int main() {
    int server = Socket(AF_INET, SOCK_STREAM, 0);
    int flags = fcntl(server, F_GETFL, 0);
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(10000);
    Bind(server, &adr, sizeof(adr));

    Listen(server, 1);
    socklen_t addrlen = sizeof(adr);
    int fd = Accept(server, &adr, addrlen);
    recv_file("r1",fd);
    recv_file("r2",fd);
    recv_file("r3",fd);
    recv_file("r4",fd);
    sleep(5);
    close(fd);
    close(server);
}