
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include "fileshare.h"



unsigned char compare_commands(const char com1[8],const char com2[8]){
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
    int n = scandir(path, &namelist, NULL, NULL);
    if(n == 0) return 0;

    int* count_files_in_msg = malloc(sizeof (int));
    char** names = split_words(message,count_files_in_msg);
    int match = 0;
    for(int i = 0;i<(*count_files_in_msg);i++){
        for(int j = 0;j<n;j++){
            if(strcmp(namelist[j]->d_name,names[i]) == 0){
                match++;
                break;
            }
        }
    }
    for(int i = 0;i<n;i++){
        free(namelist[i]);
    }
    for(int i = 0;i<(*count_files_in_msg);i++){
        free(names[i]);
    }
    free(names);
    if(match == (*count_files_in_msg)){

        free(namelist);
        free(count_files_in_msg);
        return 1;
    }
    free(count_files_in_msg);
    return 0;
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
    long long byte_size = getFileSize(path);
    char* string = getFileSizeString(byte_size);
    write(fd,string,sizeof (string));
    free(string);
    char buffer[1024];
    int total = 0;
    size_t n;
    while (1) {
        sleep(0);
        n = fread(buffer, 1, 1024, file);

        //  printf("%d\n",(int)n);
        total += send(fd,buffer,n,0);
        if(total >= byte_size) break;
    }
    printf("%d\n",total);
    fclose(file);
    while (read(fd,buffer,1) < 0);
}

void recv_list_of_files(int fd){
    write(fd,"3",1);
    char command_buff[8];
    char buff[1024];
    read(fd,command_buff,COMMAND_SIZE);
    if(compare_commands(command_buff,NEGATIVE_ANSWER)){
        return;
    }
    while (1){
        read(fd,command_buff,8);
        if(compare_commands(command_buff,POSTIVE_ANSWER)){
            recv(fd,buff,1024,0);
            printf("%s\n",buff);
        } else break;
    }
}
void send_list_of_files(int fd){
    char buff[1024];
    struct dirent ** namelist;
    read(fd,buff,1024);
    int n = scandir("./",&namelist,NULL,NULL);
    int count_files = 0;
    for(int i = 0;i<n;i++){
        if(namelist[i]->d_type == DT_REG) count_files++;
    }
    if(count_files == 0){
        write(fd,NEGATIVE_ANSWER,COMMAND_SIZE);
        return;
    }
    write(fd,POSTIVE_ANSWER,COMMAND_SIZE);
    for(int i = 0;i<n;i++){
        sleep(0);
        if(namelist[i]->d_type == DT_REG){
            write(fd,POSTIVE_ANSWER,8);
            send(fd,namelist[i]->d_name,sizeof (namelist[i]->d_name),0);
        }
    }
    write(fd,NEGATIVE_ANSWER,8);
}
void recv_file(const char *path, int fd) {
    FILE *file = fopen(path, "wb");
    char * string = malloc(20);
    read(fd,string,sizeof (string));
    long long byte_size = getFileSizeFromString(string);
    free(string);
    char buffer[1024];
    int total = 0;
    size_t n = 0;
    while (1) {
        n = recv(fd, buffer, 1024, 0);
        // printf("%d\n",(int)n);
        total += fwrite(buffer,1,n,file);
        if(total >= byte_size) break;
    }
    printf("%d\n",total);
    fclose(file);
    write(fd,"1",1);
}
char* getFileName(const char* path) {
    char* fileName = basename((char *)path);
    return fileName;
}

long long getFileSize(const char *path) {
    struct stat file_info;
    if (stat(path, &file_info) == 0) {
        long long fileSize = (long long)file_info.st_size;
        return fileSize;
    } else {
        perror("Failed to get file information");
        return -1;
    }
}

// Функция для получения размера файла в виде строки
char* getFileSizeString(long long fileSize) {
    char* fileSizeString = (char*)malloc(20 * sizeof(char));  // Максимальная длина - 20 символов
    sprintf(fileSizeString, "%lld", fileSize);
    return fileSizeString;
}

long long getFileSizeFromString(const char* fileSizeString) {
    char* endPtr;
    long long fileSize = strtoll(fileSizeString, &endPtr, 10);

    if (fileSize == 0 && endPtr == fileSizeString) {
        fprintf(stderr, "Invalid file size string: %s\n", fileSizeString);
        return -1;
    }

    return fileSize;
}