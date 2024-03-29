
#include "fwrapper.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main() {
    int fd = Socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(10000);
    Inet_pton(AF_INET, "127.0.0.1", &adr.sin_addr);
    Connect(fd, &adr, sizeof(adr));
    send_file("original1",fd);
    send_file("original2",fd);
    send_file("original3",fd);
    send_file("server.c",fd);
    sleep(2);
    close(fd);
}