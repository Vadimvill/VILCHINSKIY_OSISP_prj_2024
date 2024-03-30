//
// Created by vadim on 30.3.24.
//

#ifndef FILESHARE_FILESHARE_H
#define FILESHARE_FILESHARE_H
void send_file(const char* path,int fd);
void recv_file(const char* path,int fd);
char* concatenateStrings(const char *str1, const char *str2);
void send_list_of_files(int fd,char* path);
void recv_list_of_files(int fd);
#endif //FILESHARE_FILESHARE_H
