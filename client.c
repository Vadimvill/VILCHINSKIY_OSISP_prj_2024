#include "fwrapper.h"
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include "fileshare.h"
#include "globalconstants.h"


struct ThreadArgs {
    int fd;
    char * basePath;
    char* name;
};
void* thread_function_client(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;

    // Использование переданных аргументов
    int file_descriptor = args->fd;
    char* base_path = args->basePath;
    char* file_name = args->name;
    send_file(concatenateStrings(base_path,file_name),file_descriptor);
    pthread_exit(NULL);
}
int main() {
    // printf("Установите базовый каталог для отправки файлов\n");
    char bathPath[128] = "./\0";
    char ip[16];
    scanf("%s",ip);
    int *fd = malloc(sizeof(int) * 5);
    for (int i = 0; i < 5; i++) {
        fd[i] = Socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in adr = {0};
        adr.sin_family = AF_INET;
        adr.sin_port = htons(10000);
        Inet_pton(AF_INET, ip, &adr.sin_addr);
        Connect(fd[i], &adr, sizeof(adr));
    }
    int a;
    char buff[8];
    char msg[2048];
    while (1) {
        read(fd[0], buff, COMMAND_SIZE);
        write(fd[0], buff, COMMAND_SIZE);
        if (compareCommands(buff, GET_FILES_NAME)) send_list_of_files(fd[0], bathPath);
        if (compareCommands(buff, DOWNLOAD_FILES)) {
            read(fd[0], msg, MSG_SIZE);
            if (check_files(msg, bathPath)) {
                write(fd[0], POSTIVE_ANSWER, 8);
                int n = count_words(msg);
                char **names = split_words(msg, &n);
                pthread_t *thread_id = malloc(sizeof(pthread_t) * 4);
                int z = 1;
                int count_use_threads = 0;
                for (int i = 0; i < n; i++) {
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
                    threadArgs->basePath = bathPath;
                    threadArgs->fd = fd[z];
                    threadArgs->name = names[i];
                    int result = pthread_create(&thread_id[z - 1], NULL, thread_function_client, (void *) threadArgs);
                    z++;
                    if (result != 0) {
                        printf("Ошибка при создании потока\n");
                        return 1;
                    }
                    count_use_threads++;
                }
                for (int j = 0; j < count_use_threads; j++) {
                    pthread_join(thread_id[j], NULL);
                }


            } else write(fd[0], "00000000", 8);

        }
    }
    sleep(2);
    close(fd[0]);
}