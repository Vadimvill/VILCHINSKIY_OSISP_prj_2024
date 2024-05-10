

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include "fileshare.h"


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
    for(int i = 0;i<(*n);i++){
        free(namelist[i]);
    }
    free(namelist);
    free(n);
}

void send_file(const char *path, int fd) {
    FILE *file = fopen(path, "rb");
    long long byte_size = getFileSize(path);
    char *string = getFileSizeString(byte_size);
    write(fd, string, sizeof(string));
    free(string);
    if (byte_size == 0) {
        fclose(file);
        return;
    }
    char buffer[1024];
    int total = 0;
    size_t n = 0;
    while (1) {
        sleep(0);
        n = fread(buffer, 1, 1024, file);
        total += send(fd, buffer, n, 0);
        if (total >= byte_size) break;
    }
    printf("Total send of %s %d\n", path, total);
    fclose(file);
    while (read(fd, buffer, 1) < 0);
}

void recv_file(const char *path, int fd) {
    FILE *file = fopen(path, "wb");
    char *string = malloc(20);
    read(fd, string, sizeof(string));
    long long byte_size = getFileSizeFromString(string);
    free(string);
    if (byte_size == 0) {
        fclose(file);
        return;
    }
    char buffer[1024];
    int total = 0;
    size_t n = 0;
    while (1) {
        n = recv(fd, buffer, 1024, 0);
        total += fwrite(buffer, 1, n, file);
        if (total >= byte_size) break;
    }
    printf("Total recv of %s %d\n", path, total);
    fclose(file);
    write(fd, "1", 1);
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

char *getFileSizeString(long long fileSize) {
    char *fileSizeString = (char *) malloc(20 * sizeof(char));
    sprintf(fileSizeString, "%lld", fileSize);
    return fileSizeString;
}

long long getFileSizeFromString(const char *fileSizeString) {
    char *endPtr;
    long long fileSize = strtoll(fileSizeString, &endPtr, 10);

    if (fileSize == 0 && endPtr == fileSizeString) {
        fprintf(stderr, "Invalid file size string: %s\n", fileSizeString);
        return -1;
    }

    return fileSize;
}
