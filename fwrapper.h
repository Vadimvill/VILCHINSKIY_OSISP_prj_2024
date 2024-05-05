
#ifndef FILESHARE_FWRAPPER_H
#define FILESHARE_FWRAPPER_H
#include "sys/socket.h"
#include <netinet/in.h>

int Socket(int domain,int type,int protocol);
void Bind(int socket,struct sockaddr_in * adr,socklen_t socklen);
void Listen(int sockfg,int backlog);
int Accept(int server,struct sockaddr_in* adr, socklen_t socklen);
int Connect(int fd,struct sockaddr_in* adr,socklen_t socklen);
int Inet_pton(int af,const char* src,void *dst);
#endif //FILESHARE_FWRAPPER_H
