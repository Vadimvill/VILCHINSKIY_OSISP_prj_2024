#include "fwrapper.h"
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "fileshare.h"
#include "globalconstants.h"
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}
int main() {
    printf("Установите базовый каталог для сохранения файлов\n");
    char basePath[128];
    scanf("%s", basePath);
    int server = Socket(AF_INET, SOCK_STREAM, 0);
    fcntl(server, F_GETFL, 0);
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
        clear_input_buffer();
        char buff[8];
        if(i == 1){
            write(fd,GET_FILES_NAME,COMMAND_SIZE);
            read(fd,buff,COMMAND_SIZE);
            if(compareCommands(GET_FILES_NAME,buff))   recv_list_of_files(fd);
        }
        if(i== 2){
            printf("Введите имена файлов через пробел\n");
            char file_names[MSG_SIZE];
            scanf(" %[^\n]", file_names);
            write(fd,DOWNLOAD_FILES,COMMAND_SIZE);
            read(fd,buff,COMMAND_SIZE);
            if(compareCommands(DOWNLOAD_FILES,buff)){
                write(fd,file_names,MSG_SIZE);
                read(fd,buff,8);
                if(compareCommands(buff,POSTIVE_ANSWER)){
                    int n = count_words(file_names);
                    char** names = split_words(file_names,&n);
                    for(int b = 0;b< n;b++){
                        recv_file(concatenateStrings(basePath, names[b]), fd);
                    }
                }
            }
        }
        if(i==3){
            sleep(5);
            close(fd);
            close(server);
            exit(1);
        }
        }
    }