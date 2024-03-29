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
    FILE * file = fopen("client.c","rb");
    char buffer[1024];
    size_t n = 0;
    while ((n = fread(buffer,1,1024,file)) > 0){
        send(fd,buffer,n,0);
    }
    fclose(file);
    sleep(1);
    close(fd);
}