

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "fileshare.h"
#include "fwrapper.h"

pthread_mutex_t client_mut;
pthread_mutex_t server_mut;


unsigned char compare_commands(const char com1[8], const char com2[8]) {
    for (int i = 0; i < 8; i++) {
        if (com1[i] != com2[i]) return 0;
    }
    return 1;
}

unsigned char check_files(const char message[2048], const char *path) {

    struct dirent **namelist;
    int n = scandir(path, &namelist, NULL, NULL);
    if (n == 0) return 0;

    int *count_files_in_msg = malloc(sizeof(int));
    char **names = split_words(message, count_files_in_msg);
    int match = 0;
    for (int i = 0; i < (*count_files_in_msg); i++) {
        for (int j = 0; j < n; j++) {
            if (strcmp(namelist[j]->d_name, names[i]) == 0 && namelist[j]->d_type == 8) {
                match++;
                break;
            }
        }
    }
    for (int i = 0; i < n; i++) {
        free(namelist[i]);
    }
    for (int i = 0; i < (*count_files_in_msg); i++) {
        free(names[i]);
    }
    free(names);
    if (match == (*count_files_in_msg)) {

        free(namelist);
        free(count_files_in_msg);
        return 1;
    }
    free(count_files_in_msg);
    return 0;
}

void print_local_ip(int port) {
    struct ifaddrs *addrs, *tmp;
    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *pAddr = (struct sockaddr_in *) tmp->ifa_addr;
            char *ip = inet_ntoa(pAddr->sin_addr);

            if (strncmp(ip, "192.168.", 8) == 0 || strncmp(ip, "10.", 3) == 0) {
                printf("Server address: %s:%d\n", ip, port);
                break;
            }
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
}

char **split_words(const char message[2048], int *wordCount) {
    char **words = NULL;
    *wordCount = 0;

    char buffer[2048];
    strcpy(buffer, message);

    char *token = strtok(buffer, " \n");
    while (token != NULL) {
        (*wordCount)++;

        words = realloc(words, sizeof(char *) * (*wordCount));

        words[*wordCount - 1] = malloc(strlen(token) + 1);
        strcpy(words[*wordCount - 1], token);

        token = strtok(NULL, " \n");
    }

    return words;
}

struct dirent **scandir_reg(int *n) {
    struct dirent **namelist;
    int count = scandir("./", &namelist, NULL, NULL);
    int count_reg_files = 0;
    for (int i = 0; i < count; i++) {
        if (namelist[i]->d_type == 8) {
            count_reg_files++;
        }
    }
    struct dirent **result = malloc(sizeof(struct dirent *) * count_reg_files);
    (*n) = 0;
    for (int i = 0; i < count; i++) {
        if (namelist[i]->d_type == 8) {
            result[(*n)] = namelist[i];
            (*n)++;
        }
    }
    return result;
}

int *init_client(char ip[16]) {
    int success = 0;
    int *fd;
    do {
        fd = malloc(sizeof(int) * 8);
        for (int i = 0; i < 8; i++) {
            fd[i] = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in adr = {0};
            adr.sin_family = AF_INET;
            adr.sin_port = htons(10000);
            if (Inet_pton(AF_INET, ip, &adr.sin_addr) <= 0) {
                printf("Некорректный IP-адрес. Повторите ввод.\n");
                printf("Введите IP-адрес сервера: ");
                scanf("%s", ip);
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
    pthread_mutex_init(&client_mut,NULL);
    return fd;
}

int *init_server(int print) {
    int server_socket = Socket(AF_INET, SOCK_STREAM, 0);
    fcntl(server_socket, F_GETFL, 0);
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(10000);
    Bind(server_socket, &adr, sizeof(adr));
    Listen(server_socket, 8);
    socklen_t addrlen = sizeof(adr);
    if (print) print_local_ip(10000);
    int *fd = malloc(sizeof(int) * 9);
    fd[8] = server_socket;
    for (int i = 0; i < 8; i++) {
        fd[i] = Accept(server_socket, &adr, addrlen);
    }
    pthread_mutex_init(&server_mut,NULL);
    return fd;
}

void recv_list_of_files(int fd) {
    write(fd, "3", 1);
    char size[8];
    char buff[1024];
    read(fd, &size, 8);
    int n = atoi(size);
    for (int i = 0; i < n; i++) {
        recv(fd, buff, 1024, 0);
        printf("%s ", buff);
        write(fd, "3", 1);
    }
    printf("\n");
    lseek(fd, 0, SEEK_SET);
}

void send_list_of_files(int fd, char *path) {
    char buff[1024];
    int *n = malloc(sizeof(int));
    struct dirent **namelist = scandir_reg(n);
    read(fd, buff, 1024);
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d", (*n));
    write(fd, &buffer, 8);
    for (int i = 0; i < (*n); i++) {
        sleep(0);
        send(fd, namelist[i]->d_name, sizeof(namelist[i]->d_name), 0);
        while (read(fd, buff, 1) < 0);
    }
    for (int i = 0; i < (*n); i++) {
        free(namelist[i]);
    }
    lseek(fd, 0, SEEK_SET);
    free(namelist);
    free(n);
}



void send_file(const char *path, int fd, int command_fd) {
    pthread_mutex_lock(&server_mut);
    int file_fd = open(path, O_RDWR);
    long long byte_size = getFileSize(path);
    char str[50];
    sprintf(str, "%lld", byte_size);
    size_t size = 1024 * 64;
    char *file_bytes = malloc(size);
    send(fd, str, 50, 0);
    int total = 0;
    int total_read = 0;
    int count_iter = 0;
    while (1) {
     //   system("clear");

        char command_buff[8];
        if(total >= byte_size) {
            send(command_fd,LAST_PACKAGE,8,0);
            recv(command_fd,command_buff,8,0);
            break;
        }
        int read_size = read(file_fd,file_bytes,size);
        int send_size = send(fd,file_bytes,read_size,0);
        total+=send_size;
        send(command_fd,POSTIVE_ANSWER,8,0);
        recv(command_fd,command_buff,8,0);
    }
    free(file_bytes);
    close(file_fd);
    pthread_mutex_unlock(&server_mut);
}

void recv_file(const char *path, int fd, int command_fd) {
    pthread_mutex_lock(&client_mut);
    char str[50];
    long long byte_size;
    char *endptr;
    recv(fd, str, 50, 0);
    byte_size = strtoll(str, &endptr, 10);
    int size = 1024 * 64;
    char *file_bytes = malloc(size);
    int file_fd = open(path, O_RDWR | O_CREAT, 0644);
    int total = 0;
    int total_write = 0;
    int count_iter = 0;
    while (1) {
       // system("clear");
        char command_buf[8];
        recv(command_fd,command_buf,8,0);
        if(compare_commands(command_buf,POSTIVE_ANSWER)) {
            int recv_size = recv(fd,file_bytes,size,0);
            int write_size = write(file_fd,file_bytes,recv_size);
            total+=recv_size;
            double procent = (double)total/(double)byte_size;
            printf("%d/%lld  %d%%\n",total,byte_size,(int)(procent*100));
            send(command_fd,POSTIVE_ANSWER,8,0);
        }
        else if(compare_commands(command_buf,LAST_PACKAGE)){
            if(total >= byte_size){
                send(command_fd,POSTIVE_ANSWER,8,0);
                break;
            }
            int recv_size = recv(fd,file_bytes,size,0);
            int write_size = write(file_fd,file_bytes,recv_size);
            total+=recv_size;
            send(command_fd,POSTIVE_ANSWER,8,0);
            break;
        }
}

free(file_bytes);
close(file_fd);
    pthread_mutex_unlock(&client_mut);
}

long long getFileSize(const char *path) {
    struct stat file_info;
    if (stat(path, &file_info) == 0) {
        long long fileSize = (long long) file_info.st_size;
        return fileSize;
    } else {
        perror("Failed to get file information");
        return -1;
    }
}
