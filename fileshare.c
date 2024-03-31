
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include "fileshare.h"

unsigned char compareCommands(const char com1[8],const char com2[8]){
    for(int i = 0;i<8;i++){
        if(com1[i]!=com2[i]) return 0;
    }
    return 1;
}

unsigned char check_files(const char message[2048], const char* path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return 0;
    }
    closedir(dir);

    struct dirent **namelist;
    int n = scandir(path, &namelist, NULL, alphasort);

    if (n < 0) {
        perror("scandir");
        return 0;
    }

    int start_index = 0;
    int files_found = 0;
    int files_in_message = 0;

    int message_length = strlen(message);

    for (int i = 0; i <= message_length; i++) {
        if (message[i] == ' ' || message[i] == '\n' || message[i] == '\0') {
            int file_found = 0;

            for (int j = 0; j < n; j++) {
                if (strcmp(namelist[j]->d_name, ".") == 0 || strcmp(namelist[j]->d_name, "..") == 0) {
                    continue;  // игнорировать записи "." и ".."
                }

                int len = i - start_index;
                if (strncmp(message + start_index, namelist[j]->d_name, len) == 0 && len == strlen(namelist[j]->d_name)) {
                    file_found = 1;
                    break;
                }
            }

            if (file_found) {
                files_found++;
            } else {
                for (int j = 0; j < n; j++) {
                    free(namelist[j]);
                }
                free(namelist);

                return 0;
            }

            files_in_message++;
            start_index = i + 1;
        }
    }

    for (int j = 0; j < n; j++) {
        free(namelist[j]);
    }
    free(namelist);

    if (files_found < files_in_message) {
        return 0;  // Если количество найденных файлов меньше количества файлов в сообщении, возвращаем 0
    }

    return 1;
}
int count_words(const char message[2048]) {
    int wordCount = 0;
    int inWord = 0;

    for (int i = 0; i < 2048; i++) {
        if (message[i] == ' ' || message[i] == '\n' || message[i] == '\0') {
            if (inWord) {
                inWord = 0;
                wordCount++;
            }
        } else {
            inWord = 1;
        }
    }

    return wordCount;
}
char* concatenateStrings(const char *str1, const char *str2) {
    char *result = malloc(strlen(str1) + strlen(str2) + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation error\n");
    }
    sprintf(result, "%s%s", str1, str2);
    return result;
}
char** split_words(const char message[2048], int* wordCount) {
    char** words = NULL;
    *wordCount = 0;

    // Копируем сообщение во временный буфер для обработки
    char buffer[2048];
    strcpy(buffer, message);

    // Используем strtok для разделения строки на слова
    char* token = strtok(buffer, " \n");
    while (token != NULL) {
        (*wordCount)++;

        // Увеличиваем размер массива слов
        words = realloc(words, sizeof(char*) * (*wordCount));

        // Выделяем память для хранения копии слова и копируем его
        words[*wordCount - 1] = malloc(strlen(token) + 1);
        strcpy(words[*wordCount - 1], token);

        // Получаем следующее слово
        token = strtok(NULL, " \n");
    }

    return words;
}
void send_file(const char *path, int fd) {
    FILE *file = fopen(path, "rb");
    printf("file %s is open\n", path);
    char buffer[1024];
    int total = 0;
    size_t n = 0;
    while (1) {
        sleep(0);
        n = fread(buffer, 1, 1024, file);

        //  printf("%d\n",(int)n);
        total += send(fd,buffer,n,0);
        if(n < 1024) break;
    }
    printf("%d\n",total);
    fclose(file);
    while (read(fd,buffer,1) < 0);
}
void recv_list_of_files(int fd){
    write(fd,"3",1);
    char size;
    char buff[1024];
    read(fd,&size,1);
    for(int i = 0;i<(int)size-2;i++){
        recv(fd,buff,1024,0);
        printf("%s\n",buff);
        write(fd,"3",1);
    }
}
void send_list_of_files(int fd,char* path){
    char buff[1024];
    struct dirent ** namelist;
    read(fd,buff,1024);
    char n = scandir(path,&namelist,NULL,NULL);
    write(fd, &n, 1);
    for(int i = 2;i<n;i++){
        sleep(0);
        send(fd,namelist[i]->d_name,sizeof (namelist[i]->d_name),0);
        while (read(fd,buff,1) < 0);
    }
}
void recv_file(const char *path, int fd) {
    FILE *file = fopen(path, "wb");
    printf("file %s is open\n", path);
    char buffer[1024];
    int total = 0;
    size_t n = 0;
    while (1) {
        n = recv(fd, buffer, 1024, 0);
        // printf("%d\n",(int)n);
        total += fwrite(buffer,1,n,file);
        if(n < 1024) break;
    }
    printf("%d\n",total);
    fclose(file);
    write(fd,"1",1);
}