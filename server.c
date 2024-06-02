
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
int* copy_fd;
struct ThreadArgs {
    int fd;
    char *name;
    int command_fd;
};
void *thread_function_send(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *) arg;
    int file_descriptor = args->fd;
    char *file_name = args->name;
    int command_fd = args->command_fd;
    send_file(file_name, file_descriptor,command_fd);
    pthread_exit(NULL);
}

void *thread_function_recv(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *) arg;
    int file_descriptor = args->fd;
    char *file_name = args->name;
    int command_fd = args->command_fd;
    recv_file(file_name, file_descriptor,command_fd);
    pthread_exit(NULL);
}


void menu_close_server(int *fd, int server) {
    sleep(1);
    for (int i = 0; i < 8; i++) {
        close(fd[i]);
    }
    close(fd[8]);
    printf("closet\n");
    sleep(1);

}

void menu_close_client(int *fd) {
    for (int i = 0; i < 8; i++) {
        close(fd[i]);
    }
    printf("closet client\n");
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
        int command_fd_index = 4;
        pthread_t *array = malloc(sizeof(pthread_t)*4);
        for (int i = 0; i < (*n); i++) {
            if(count_use_threads == 4){
                for(int k = 0;k<4;k++){
                    pthread_join(array[k],NULL);
                }
                count_use_threads = 0;
                command_fd_index=4;
            }
            struct ThreadArgs threadArgs;
            threadArgs.name = names[i];
            threadArgs.fd = fd[count_use_threads];
            threadArgs.command_fd = fd[command_fd_index];
            int result = pthread_create(&array[count_use_threads],NULL, thread_function_recv,&threadArgs);
            count_use_threads++;
            command_fd_index++;
            sleep(0);
        }
        for(int z = 0;z<count_use_threads;z++){
            pthread_join(array[z],NULL);
        }
        free(array);
    }
    for(int i = 0;i<(*n);i++){
        free(names[i]);
    }
    free(names);
    free(n);
}

void menu_send_file(int *fd, char *base_path) {
    char msg_buff[MSG_SIZE];

    read(fd[0], msg_buff, MSG_SIZE);
    int *n = malloc(sizeof(int));
    char **names = split_words(msg_buff, n);


    if (check_files(msg_buff, "./")) {
        write(fd[0], POSTIVE_ANSWER, COMMAND_SIZE);
        int count_use_threads = 0;
        int command_fd_index = 4;
        pthread_t *array = malloc(sizeof(pthread_t)*4);
        for (int i = 0; i < (*n); i++) {
            if(count_use_threads == 4){
                for(int c = 0;c<4;c++){
                    pthread_join(array[c],NULL);
                }
                count_use_threads = 0;
                command_fd_index = 4;
            }
            struct ThreadArgs threadArgs;
            threadArgs.name = names[i];
            threadArgs.fd = fd[count_use_threads];
            threadArgs.command_fd = fd[command_fd_index];
            int result = pthread_create(&array[count_use_threads],NULL, thread_function_send,&threadArgs);
            count_use_threads++;
            command_fd_index++;
            sleep(0);
        }
        for(int c = 0;c<count_use_threads;c++){
            pthread_join(array[c],NULL);
        }
        free(array);
        for(int i = 0;i<(*n);i++){
            free(names[i]);
        }
        free(names);
        free(n);
    } else write(fd[0], NEGATIVE_ANSWER, COMMAND_SIZE);
}
void clean_up_server(int sig) {
    menu_close_server(copy_fd, copy_fd[8]);
    free(copy_fd);
    exit(0);
}
void clean_up_client(int sig) {
    menu_close_client(copy_fd);
    free(copy_fd);
    exit(0);
}
int client(char* base_path_server,int ppid) {
    printf("Установите базовый каталог для сохранения файлов\n");
    char base_path[128];
    while (1){
        scanf("%s", base_path);
        if(strncmp(base_path,base_path_server,128) == 0){
            printf("Не удалось изменить рабочий каталог т.к у сервера такой же.\n");
            continue;
        }
        if (chdir(base_path) == 0) {
            printf("Рабочий каталог изменен на: %s\n", base_path);
            break;
        } else {
            printf("Не удалось изменить рабочий каталог.\n");
        }
    }
    char *ip = malloc(16);
    printf("Введите IP-адрес сервера: ");
    scanf("%s", ip);
    int *fd;
    signal(SIGTERM,clean_up_client);
    fd = init_client(ip);
    copy_fd = fd;
    while (1) {
        char *choise = malloc(1);
        printf("1.Скачать файлы\n2.Список файлов\n3.Закрыть соединение\n");
        scanf(" %c", choise);
        char buff[8];
        write(fd[0], choise, 1);
        if ((*choise) == '1'){
            menu_recv_files(fd);
            menu_close_client(fd);
            sleep(3);
            fd = init_client(ip);
            copy_fd = fd;
        }
        if((*choise) == '2'){
            recv_list_of_files(fd[0]);
            menu_close_client(fd);
            sleep(3);
            fd = init_client(ip);
            copy_fd = fd;
        }
        if ((*choise) == '3'){
            free(choise);
            menu_close_client(fd);
            free(fd);
            sleep(2);
            kill(ppid,SIGTERM);
            exit(1);
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
            printf("Рабочий каталог изменен на: %s\n", base_path);
            break;
        } else {
            printf("Не удалось изменить рабочий каталог.\n");
        }
    }
    chdir(cwd);
    int ppid = getpid();
    int pid = fork();
    if (pid == 0) {
        client(base_path,ppid);
        exit(0);
    }
    signal(SIGTERM,clean_up_server);
    chdir(base_path);
    int*fd;
    fd = init_server(1);
    copy_fd = fd;
    while (1) {
        char *choise = malloc(1);
        read(fd[0], choise, 1);
        char buff[8];
        if ((*choise) == '1'){
            menu_send_file(fd,base_path);
            menu_close_server(fd,fd[8]);
            fd = init_server(0);
            copy_fd = fd;
        }
        if((*choise) == '2'){
            send_list_of_files(fd[0],"./");
            menu_close_server(fd,fd[8]);
            fd = init_server(0);
            copy_fd = fd;
        }
        if ((*choise) == '3'){
            free(choise);
            menu_close_server(fd, fd[8]);
            free(fd);
            kill(pid,SIGTERM);
            exit(1);
        }
    }
}

int main() {
    server();
}