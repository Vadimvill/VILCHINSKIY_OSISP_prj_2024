
#include "fwrapper.h"
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include "fileshare.h"
#include "globalconstants.h"
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
    char buff[8];
    char msg[2048];
    while (1){
        read(fd,buff,COMMAND_SIZE);
        write(fd,buff,COMMAND_SIZE);
        if(compareCommands(buff,GET_FILES_NAME)) send_list_of_files(fd,bathPath);
        if(compareCommands(buff,DOWNLOAD_FILES)){
            read(fd,msg,MSG_SIZE);
            if(check_files(msg,bathPath)){
                write(fd,POSTIVE_ANSWER,8);
                int n = count_words(msg);
                char ** names = split_words(msg,&n);
                for(int i = 0;i<n;i++){
                    send_file(concatenateStrings(bathPath,names[i]),fd);
                }
            } else write(fd,"00000000",8);

        }
    }
    sleep(2);
    close(fd);
}