
#include "fwrapper.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "fileshare.h"
extern int scandir(const char *dirp, struct dirent ***namelist,
                   int (*filter)(const struct dirent*),
                   int (*compar)(const struct dirent**, const struct dirent**));
int main() {
    printf("Установите базовый каталог для отправки файлов\n");
    char bathPath[128];
    scanf("%s",bathPath);
    int fd = Socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(10000);
    Inet_pton(AF_INET, "127.0.0.1", &adr.sin_addr);
    Connect(fd, &adr, sizeof(adr));
    while (1){
        send_list_of_files(fd,bathPath);
    }
    sleep(2);
    close(fd);
}