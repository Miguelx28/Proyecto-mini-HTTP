#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "server.h"


int create_server(int port){
    int fd;
    int option = 1;
    struct sockaddr_in server_addr;

    // Crear socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0){
        perror("socket");
        return -1;
    }

    //reiniciar sin esperar
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0){
        perror("setsockopt");
        close(fd);
        return -1;
    }

    // llenar la direccion con cualquier ip y puerto indicado
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        close(fd);
        return -1;
    }
    if(listen(fd, 128) < 0){
        perror("listen");
        close(fd);
        return -1;
    }
    return fd;
}  