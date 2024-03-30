
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include "fileshare.h"
char* concatenateStrings(const char *str1, const char *str2) {
    char *result = malloc(strlen(str1) + strlen(str2) + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation error\n");
    }
    sprintf(result, "%s%s", str1, str2);
    return result;
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