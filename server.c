#include "fwrapper.h"
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "fileshare.h"

int main() {
    printf("Установите базовый каталог для сохранения файлов\n");
    char bathPath[128];
    scanf("%s",bathPath);
    int server = Socket(AF_INET, SOCK_STREAM, 0);
    int flags = fcntl(server, F_GETFL, 0);
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(10000);
    Bind(server, &adr, sizeof(adr));
    Listen(server, 1);
    socklen_t addrlen = sizeof(adr);
    int fd = Accept(server, &adr, addrlen);
    while (1){
        int i = -1;
        printf("1) Получить список файлов\n2) Скачать файл\n3) Закрыть соеденение\n");
        scanf("%d",&i);
        if(i == 1){
            recv_list_of_files(fd);
        }
        if(i==3){
            sleep(5);
            close(fd);
            close(server);
            exit(1);
        }
        }
    }