

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
#include <signal.h>

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
    char *basePath;
    char *name;
};

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

void menu_recv_files(int fd) {
    char command_buff[COMMAND_SIZE];
    char msg_buff[MSG_SIZE];

    printf("Введите именя файлов через пробел\n");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // очищаем оставшиеся символы новой строки во входном потоке
    }

    fgets(msg_buff, sizeof(msg_buff), stdin);

    int *n = malloc(sizeof(int));
    char **names = split_words(msg_buff, n);

    write(fd, msg_buff, MSG_SIZE);

    read(fd, command_buff, COMMAND_SIZE);

    if (compareCommands(command_buff, POSTIVE_ANSWER)) {
        for (int i = 0; i < (*n); i++) {
            recv_file(names[i], fd);
        }
    }
}

void menu_send_file(int fd, char *base_path) {
    char msg_buff[MSG_SIZE];
// отправка файлов
    read(fd, msg_buff, MSG_SIZE);
    int *n = malloc(sizeof(int));
    char **names = split_words(msg_buff, n);


    for (int i = 0; i < (*n); i++) {
        printf("%s\n", names[i]);
    }

    if (check_files(msg_buff, "./")) {
        write(fd, POSTIVE_ANSWER, COMMAND_SIZE);
        for (int i = 0; i < (*n); i++) {
            send_file(names[i],fd);
        }
    } else write(fd, NEGATIVE_ANSWER, COMMAND_SIZE);
}

void *thread_function_server(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *) arg;
    int file_descriptor = args->fd;
    char *base_path = args->basePath;
    char *file_name = args->name;
    //recv_file(concatenateStrings(base_path, file_name), file_descriptor);
    pthread_exit(NULL);
}

void *thread_function_client(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *) arg;

    // Использование переданных аргументов
    int file_descriptor = args->fd;
    char *base_path = args->basePath;
    char *file_name = args->name;
    send_file(concatenateStrings(base_path, file_name), file_descriptor);
    pthread_exit(NULL);
}

int client() {
    printf("Установите базовый каталог для сохранения файлов\n");
    char base_path[128];
    scanf("%s", base_path);
    if (chdir(base_path) == 0) {
        printf("Рабочая директория изменена на: %s\n", base_path);
    } else {
        printf("Не удалось изменить рабочую директорию.\n");
    }
    printf("Введите IP: ");
    char ip[16];
    scanf("%s", ip);
    int *fd = malloc(sizeof(int) * 5);
    for (int i = 0; i < 5; i++) {
        fd[i] = Socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in adr = {0};
        adr.sin_family = AF_INET;
        adr.sin_port = htons(10000);
        Inet_pton(AF_INET, ip, &adr.sin_addr);
        Connect(fd[i], &adr, sizeof(adr));
    }


    while (1) {
        char *choise = malloc(1);
        printf("1.Скачать файлы\n2.Список файлов\n3.Закрыть соединение");
        scanf("%c", choise);
        write(fd[0], choise, 1);
        if ((*choise) == '1') menu_recv_files(fd[0]);
        if((*choise) == '2') recv_list_of_files(fd[0]);
        if ((*choise) == '3') menu_close_client(fd);
    }


    return 1;
}

int server() {
    printf("Установите базовый каталог для отправки файлов\n");
    char base_path[128] = "recv/\0";
    scanf("%s", base_path);
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
        client();
        exit(0);  // Завершение дочернего процесса
    }
    if (chdir(base_path) == 0) {
        printf("Рабочая директория изменена на: %s\n", base_path);
    } else {
        printf("Не удалось изменить рабочую директорию.\n");
    }
    int *fd = malloc(sizeof(int) * 5);
    for (int i = 0; i < 5; i++) {
        fd[i] = Accept(server_socket, &adr, addrlen);
    }
    while (1) {
        char *choise = malloc(1);
        read(fd[0], choise, 1);
        if ((*choise) == '1') menu_send_file(fd[0],base_path);
        if((*choise) == '2') send_list_of_files(fd[0],"./");
        if ((*choise) == '3') menu_close_server(fd, server_socket);
    }

    char command_buff[COMMAND_SIZE];


    return 1;
}

int main() {
    server();
}
