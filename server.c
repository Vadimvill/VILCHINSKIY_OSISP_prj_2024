
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
struct ThreadArgs {
    int fd;
    char *name;
};
void *thread_function_send(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *) arg;
    int file_descriptor = args->fd;
    char *file_name = args->name;
    send_file(file_name, file_descriptor);
    pthread_exit(NULL);
}

void *thread_function_recv(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *) arg;
    int file_descriptor = args->fd;
    char *file_name = args->name;
    recv_file(file_name, file_descriptor);
    pthread_exit(NULL);
}
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


void menu_close_server(int *fd, int server) {
    sleep(1);
    close(server);
    for (int i = 0; i < 5; i++) {
        close(fd[i]);
    }
    free(fd);
    exit(1);
}

void menu_close_client(int *fd) {
    for (int i = 0; i < 5; i++) {
        close(fd[i]);
    }
    free(fd);
    exit(1);
}

void menu_recv_files(int *fd) {
    char command_buff[COMMAND_SIZE];
    char msg_buff[MSG_SIZE];

    printf("Введите именя файлов через пробел\n");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {

    }

    fgets(msg_buff, sizeof(msg_buff), stdin);

    int *n = malloc(sizeof(int));
    char **names = split_words(msg_buff, n);

    write(fd[0], msg_buff, MSG_SIZE);

    read(fd[0], command_buff, COMMAND_SIZE);

    if (compare_commands(command_buff, POSTIVE_ANSWER)) {
        int count_use_threads = 0;
        pthread_t *array = malloc(sizeof(pthread_t)*4);
        for (int i = 0; i < (*n); i++) {
            if(count_use_threads == 4){
                for(int k = 0;k<4;k++){
                    pthread_join(array[k],NULL);
                }
                count_use_threads = 0;
            }
            struct ThreadArgs threadArgs;
            threadArgs.name = names[i];
            threadArgs.fd = fd[count_use_threads+1];
            int result = pthread_create(&array[count_use_threads],NULL, thread_function_recv,&threadArgs);
            count_use_threads++;
            sleep(0);
          //  printf("Count use threads in client %d \n",count_use_threads);
        }
        for(int z = 0;z<count_use_threads;z++){
            pthread_join(array[z],NULL);
        }
    }
}

void menu_send_file(int *fd, char *base_path) {
    char msg_buff[MSG_SIZE];

    read(fd[0], msg_buff, MSG_SIZE);
    int *n = malloc(sizeof(int));
    char **names = split_words(msg_buff, n);


    if (check_files(msg_buff, "./")) {
        write(fd[0], POSTIVE_ANSWER, COMMAND_SIZE);
        int count_use_threads = 0;
        pthread_t *array = malloc(sizeof(pthread_t)*4);
        for (int i = 0; i < (*n); i++) {
            if(count_use_threads == 4){
                for(int c = 0;c<4;c++){
                    pthread_join(array[c],NULL);
                }
                count_use_threads = 0;
            }
            struct ThreadArgs threadArgs;
            threadArgs.name = names[i];
            threadArgs.fd = fd[count_use_threads+1];
            int result = pthread_create(&array[count_use_threads],NULL, thread_function_send,&threadArgs);
            count_use_threads++;
            sleep(0);
          //  printf("Count use threads in server %d \n",count_use_threads);
        }
        for(int c = 0;c<count_use_threads;c++){
            pthread_join(array[c],NULL);
        }
    } else write(fd[0], NEGATIVE_ANSWER, COMMAND_SIZE);
}

int client(char* base_path_server) {
    printf("Установите базовый каталог для сохранения файлов\n");
    char base_path[128];
    while (1){
        scanf("%s", base_path);
        if(strncmp(base_path,base_path_server,128) == 0){
            printf("Не удалось изменить рабочую директорию т.к у сервера такая же.\n");
            continue;
        }
        if (chdir(base_path) == 0) {
            printf("Рабочая директория изменена на: %s\n", base_path);
            break;
        } else {
            printf("Не удалось изменить рабочую директорию.\n");
        }
    }
    char ip[16];
    int success = 0;
    int *fd;
    do {
        printf("Введите IP-адрес сервера: ");
        scanf("%s", ip);
        fd = malloc(sizeof(int) * 5);
        for (int i = 0; i < 5; i++) {
            fd[i] = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in adr = {0};
            adr.sin_family = AF_INET;
            adr.sin_port = htons(10000);
            if (Inet_pton(AF_INET, ip, &adr.sin_addr) <= 0) {
                printf("Некорректный IP-адрес. Повторите ввод.\n");
                success = 0;
                free(fd);
                break;
            }
            if (Connect(fd[i], &adr, sizeof(adr)) < 0) {
                printf("Не удалось подключиться к серверу. Повторите ввод.\n");
                success = 0;
                free(fd);
                break;
            }
            success = 1;
        }

    } while (!success);


    while (1) {
        char *choise = malloc(1);
        printf("1.Скачать файлы\n2.Список файлов\n3.Закрыть соединение\n");
        scanf(" %c", choise);
        write(fd[0], choise, 1);
        if ((*choise) == '1'){
            menu_recv_files(fd);
        }
        if((*choise) == '2'){
            recv_list_of_files(fd[0]);
        }
        if ((*choise) == '3'){
            menu_close_client(fd);
        }
    }
}

int server() {
    printf("Установите базовый каталог для отправки файлов\n");
    char base_path[128] = "recv/\0";
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char target[256];
        strcpy(target, cwd);
        printf("Текущая директория: %s\n", target);
    } else {
        perror("getcwd() error");
        return 1;
    }
    while (1){
        scanf("%s", base_path);
        if (chdir(base_path) == 0) {
            printf("Рабочая директория изменена на: %s\n", base_path);
            break;
        } else {
            printf("Не удалось изменить рабочую директорию.\n");
        }
    }
    chdir(cwd);
    int server_socket = Socket(AF_INET, SOCK_STREAM, 0);

    fcntl(server_socket, F_GETFL, 0);
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(10000);
    Bind(server_socket, &adr, sizeof(adr));
    Listen(server_socket, 5);
    socklen_t addrlen = sizeof(adr);
    print_local_ip(10000);
    int pid = fork();
    if (pid == 0) {
        client(base_path);
        exit(0);
    }
    chdir(base_path);
    int *fd = malloc(sizeof(int) * 5);
    for (int i = 0; i < 5; i++) {
        fd[i] = Accept(server_socket, &adr, addrlen);
    }
    while (1) {
        char *choise = malloc(1);
        read(fd[0], choise, 1);
        if ((*choise) == '1') menu_send_file(fd,base_path);
        if((*choise) == '2') send_list_of_files(fd[0],"./");
        if ((*choise) == '3') menu_close_server(fd, server_socket);
    }
}

int main() {
    server();
}
