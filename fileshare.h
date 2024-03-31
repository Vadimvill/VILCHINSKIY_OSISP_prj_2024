//
// Created by vadim on 30.3.24.
//
#include "globalconstants.h"
#ifndef FILESHARE_FILESHARE_H
#define FILESHARE_FILESHARE_H
void send_file(const char* path,int fd);
void recv_file(const char* path,int fd);
char* concatenateStrings(const char *str1, const char *str2);
void send_list_of_files(int fd,char* path);
void recv_list_of_files(int fd);
unsigned char compareCommands(const char com1[8],const char com2[8]);
unsigned char check_files(const char message[2048], const char* path);
int count_words(const char message[2048]);
char** split_words(const char message[2048], int* wordCount);
char* concatenateStrings(const char *str1, const char *str2);
#endif //FILESHARE_FILESHARE_H
