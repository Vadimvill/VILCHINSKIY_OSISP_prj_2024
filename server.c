#include "fwrapper.h"
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include "fileshare.h"
#include "globalconstants.h"
#include <ifaddrs.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>

void print_local_ip(int port) {
    struct ifaddrs *addrs, *tmp;
    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *pAddr = (struct sockaddr_in *) tmp->ifa_addr;
            char *ip = inet_ntoa(pAddr->sin_addr);

            // Проверяем, что IP-адрес принадлежит локальной сети
            if (strncmp(ip, "192.168.", 8) == 0 || strncmp(ip, "10.", 3) == 0) {
                printf("Server address: %s:%d\n", ip, port);
                break; // Мы нашли IP-адрес в локальной сети, поэтому выходим из цикла
            }
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}
struct ThreadArgs {
    int fd;
    char * basePath;
    char* name;
};
void* thread_function_server(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    int file_descriptor = args->fd;
    char* base_path = args->basePath;
    char* file_name = args->name;
    recv_file(concatenateStrings(base_path, file_name), file_descriptor);
    pthread_exit(NULL);
}
int main() {
    printf("Установите базовый каталог для сохранения файлов\n");
    char basePath[128] = "recv/\0";
    scanf("%s", basePath);
    int server = Socket(AF_INET, SOCK_STREAM, 0);

    fcntl(server, F_GETFL, 0);
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(10000);
    Bind(server, &adr, sizeof(adr));
    Listen(server, 5);
    socklen_t addrlen = sizeof(adr);
    int *fd = malloc(sizeof (int)* 5);
    print_local_ip(10000);
    int pid = fork();
    if(pid == 0){
        int result = execve("./client",NULL,NULL);
        sleep(15);
    }
    for(int i = 0;i<5;i++){
        fd[i] = Accept(server, &adr, addrlen);
    }
    for(int i = 0;i<5;i++) printf("%d\n",fd[i]);
    while (1){
        int i = -1;
        printf("1) Получить список файлов\n2) Скачать файл\n3) Закрыть соеденение\n");
        scanf("%d",&i);
        clear_input_buffer();
        char buff[8];
        if(i == 1){
            write(fd[0],GET_FILES_NAME,COMMAND_SIZE);
            read(fd[0],buff,COMMAND_SIZE);
            if(compareCommands(GET_FILES_NAME,buff))   recv_list_of_files(fd[0]);
        }
        if(i== 2){
            printf("Введите имена файлов через пробел\n");
            char file_names[MSG_SIZE];
            scanf(" %[^\n]", file_names);
            write(fd[0],DOWNLOAD_FILES,COMMAND_SIZE);
            read(fd[0],buff,COMMAND_SIZE);
            if(compareCommands(DOWNLOAD_FILES,buff)){
                write(fd[0],file_names,MSG_SIZE);
                read(fd[0],buff,8);
                if(compareCommands(buff,POSTIVE_ANSWER)) {
                    int n = count_words(file_names);
                    char **names = split_words(file_names, &n);
                    pthread_t* thread_id = malloc(sizeof(pthread_t) * 4);
                    int z = 1;
                    int count_use_threads = 0;
                    for (int b = 0; b < n; b++) {
                        if(count_use_threads == 4){
                            z = 1;
                            for(int j = 0;j<4;j++){
                                pthread_join(thread_id[j], NULL);
                            }
                            free(thread_id);
                            thread_id = malloc(sizeof(pthread_t) * 4);
                            count_use_threads = 0;
                            z = 1;
                        }

                        struct ThreadArgs* threadArgs = malloc(sizeof(struct ThreadArgs));
                        threadArgs->basePath = basePath;
                        threadArgs->fd = fd[z];
                        threadArgs->name = names[b];
                        int result = pthread_create(&thread_id[z - 1], NULL, thread_function_server,
                                                    (void *) threadArgs);
                        z++;
                        count_use_threads++;

                        if (result != 0) {
                            return 1;
                        }
                    }
                    for (int j = 0; j < count_use_threads; j++) {
                        pthread_join(thread_id[j], NULL);
                    }
                }

            }
        }
        if(i==3){
            sleep(5);
            for(int j = 0;j<5;j++){
                close(fd[j]);
            }
            close(server);
            exit(1);
        }
    }
}
