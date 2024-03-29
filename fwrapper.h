
#ifndef FILESHARE_FWRAPPER_H
#define FILESHARE_FWRAPPER_H
#include "sys/socket.h"
#include <netinet/in.h>

int Socket(int domain,int type,int protocol);
void Bind(int socket,struct sockaddr_in * adr,socklen_t socklen);
void Listen(int sockfg,int backlog);
int Accept(int server,struct sockaddr_in* adr, socklen_t socklen);
void Connect(int fd,struct sockaddr_in* adr,socklen_t socklen);
void Inet_pton(int af,const char* src,void *dst);
void send_file(const char* path,int fd);
void recv_file(const char* path,int fd);
#endif //FILESHARE_FWRAPPER_H
