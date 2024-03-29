#include "fwrapper.h"
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main() {
    int server = Socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(10000);
    Bind(server, &adr, sizeof(adr));
    Listen(server, 1);
    socklen_t addrlen = sizeof(adr);
    int fd = Accept(server, &adr, addrlen);
    FILE * file = fopen("recvclient.c","wb");
    char buffer[1024];
    size_t n = 0;
    while ((n = recv(fd,buffer,1024,0)) > 0){
        fwrite(buffer,1,n,file);
    }
    fclose(file);
    sleep(3);
    close(fd);
    close(server);
}