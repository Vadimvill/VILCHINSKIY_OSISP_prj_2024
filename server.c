
#include "fwrapper.c"
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include "fileshare.c"
#include "globalconstants.c"
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>
struct ThreadArgs {
    int fd;
    char *basePath;
    char *name;
};
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

void *thread_function_server(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *) arg;
    int file_descriptor = args->fd;
    char *base_path = args->basePath;
    char *file_name = args->name;
    recv_file(file_name, file_descriptor);
    pthread_exit(NULL);
}

void *thread_function_client(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *) arg;

    // Использование переданных аргументов
    int file_descriptor = args->fd;
    char *base_path = args->basePath;
    char *file_name = args->name;
    send_file(file_name, file_descriptor);
    pthread_exit(NULL);
}

void clear_input_buffer() {
    int c;
    c = getchar();
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
        // очищаем оставшиеся символы новой строки во входном потоке
    }

    fgets(msg_buff, sizeof(msg_buff), stdin);

    int *n = malloc(sizeof(int));
    char **names = split_words(msg_buff, n);

    write(fd[0], msg_buff, MSG_SIZE);

    read(fd[0], command_buff, COMMAND_SIZE);

    if (compare_commands(command_buff, POSTIVE_ANSWER)) {
        pthread_t *thread_id = malloc(sizeof(pthread_t) * 4);
        int z = 1;
        int count_use_threads = 0;
        for (int i = 0; i < (*n); i++) {
            if (count_use_threads == 4) {
                for (int j = 0; j < 4; j++) {
                    pthread_join(thread_id[j], NULL);
                }
                free(thread_id);
                thread_id = malloc(sizeof(pthread_t) * 4);
                count_use_threads = 0;
                z = 1;
            }
            struct ThreadArgs *threadArgs = malloc(sizeof(struct ThreadArgs));
            threadArgs->basePath = " ";
            threadArgs->fd = fd[z];
            threadArgs->name = names[i];
            int result = pthread_create(&thread_id[z - 1], NULL, thread_function_client, (void *) threadArgs);
            z++;
            if (result != 0) {
                printf("Ошибка при создании потока\n");
            }
            count_use_threads++;
        }
        for (int j = 0; j < count_use_threads; j++) {
            pthread_join(thread_id[j], NULL);
        }

    }
}

void menu_send_file(int *fd, char *base_path) {
    char msg_buff[MSG_SIZE];

    read(fd[0], msg_buff, MSG_SIZE);
    int *n = malloc(sizeof(int));

    char **names = split_words(msg_buff, n);


    for (int i = 0; i < (*n); i++) {
        printf("%s\n", names[i]);
    }

    if (check_files(msg_buff, base_path)) {
        write(fd[0], POSTIVE_ANSWER, COMMAND_SIZE);
        pthread_t* thread_id = malloc(sizeof(pthread_t) * 4);
        int z = 1;
        int count_use_threads = 0;
        for (int b = 0; b < (*n); b++) {
            if(count_use_threads == 4){
                for(int j = 0;j<4;j++){
                    pthread_join(thread_id[j], NULL);
                }
                free(thread_id);
                thread_id = malloc(sizeof(pthread_t) * 4);
                count_use_threads = 0;
                z = 1;
            }

            struct ThreadArgs* threadArgs = malloc(sizeof(struct ThreadArgs));
            threadArgs->basePath = "";
            threadArgs->fd = fd[z];
            threadArgs->name = names[b];
            int result = pthread_create(&thread_id[z - 1], NULL, thread_function_server,
                                        (void *) threadArgs);
            if(result == -1){

            }
            z++;
            count_use_threads++;

        }
        for (int j = 0; j < count_use_threads; j++) {
            pthread_join(thread_id[j], NULL);
        }
    } else write(fd[0], NEGATIVE_ANSWER, COMMAND_SIZE);
}

int client() {
    system("pwd");
    printf("Установите базовый каталог для сохранения файлов\n");
    clear_input_buffer();
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
   clear_input_buffer();
    while (1) {
        char *choise = malloc(1);
        printf("1.Скачать файлы\n2.Список файлов\n3.Закрыть соединение\n");
        scanf("%c", choise);
        write(fd[0], choise, 1);
        if ((*choise) == '1') menu_recv_files(fd);
        if((*choise) == '2') recv_list_of_files(fd[0]);
        if ((*choise) == '3') menu_close_client(fd);
    }


    return 1;
}

int server() {
    printf("Установите базовый каталог для отправки файлов\n");
    char base_path[128];
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
        if ((*choise) == '1') menu_send_file(fd,base_path);
        if((*choise) == '2') send_list_of_files(fd[0]);
        if ((*choise) == '3') menu_close_server(fd, server_socket);
    }

    char command_buff[COMMAND_SIZE];


    return 1;
}

int main() {
    client();
}