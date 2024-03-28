#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <string.h>

#define NUM_THREADS 2
#define PORT1 8079
#define PORT2 8080
#define BUFFER_SIZE 1024
void print_local_ip(int port) {
    struct ifaddrs *addrs, *tmp;
    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            char *ip = inet_ntoa(pAddr->sin_addr);

            // Проверяем, что IP-адрес принадлежит локальной сети
            if (strncmp(ip, "192.168.", 8) == 0 || strncmp(ip, "10.", 3) == 0) {
                printf("Server address: %s:%d\n", ip,port);
                break; // Мы нашли IP-адрес в локальной сети, поэтому выходим из цикла
            }
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
}
void *client(void *arg) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    int port = *((int *) arg); // Получаем порт из аргумента

    // Создание TCP сокета
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Socket creation error\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, "192.168.43.135", &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address/ Address not supported\n");
        exit(EXIT_FAILURE);
    }
    int con = 0;
    for (int i = 0; i < 10; i++) {
        if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            printf("%s","try to connect\n");
        } else {
            con = 1;
            break;
        }
        sleep(5);
    }
    if (!con) {
        exit(EXIT_FAILURE);
    }
    // Открытие файла для записи
    FILE *outfile = fopen("received", "wb");
    if (!outfile) {
        fprintf(stderr, "file open\n");
        exit(EXIT_FAILURE);
    }

    // Чтение данных от сервера и запись в файл
    while ((valread = read(sock, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, 1, valread, outfile);
    }
    fclose(outfile);
    close(sock);
    return NULL;
}

void *server(void *arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    int port = *((int *) arg); // Получаем порт из аргумента

    // Создание TCP сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Привязка сокета к адресу и порту
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    print_local_ip(port);
    // Ожидание клиентского подключения
    if (listen(server_fd, 20) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Принятие подключения
    if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("set past\n");
    char path[BUFFER_SIZE];
    scanf("%s", path);

    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("file open");
        exit(EXIT_FAILURE);
    }

    // Чтение из файла и отправка клиенту
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(new_socket, buffer, bytes_read, 0);
    }
    fclose(file);
    close(new_socket);
    close(server_fd);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;

    // Создание потоков

    int port1 = PORT1;
    int port2 = PORT2;

    rc = pthread_create(&threads[0], NULL, server, (void *) &port1);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_create(&threads[1], NULL, client, (void *) &port2);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }


    // Ожидание завершения потоков
    for (t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
        printf("Thread %ld has finished\n", t);
    }

    pthread_exit(NULL);
}

