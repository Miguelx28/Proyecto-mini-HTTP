#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"

#define PORT 8080

// respuesta fija, solo para probar la fase 1 
const char *response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 5\r\n"
    "Connection: close\r\n"
    "\r\n"
    "hola\n";

int main(int argc, char *argv[])
{
    int port = PORT;
    int server, client;
    char buf[4096];
    int n;

    if (argc > 1)
        port = atoi(argv[1]);

    server = create_server(port);
    if (server < 0)
        return 1;
        
    printf("Servidor escuchando en el puerto %d\n", port);
    while (1) {
        // esperar a que llegue un cliente 
        client = accept(server, NULL, NULL);
        if (client < 0) {
            perror("accept");
            continue;
        }
        printf("Cliente conectado\n");
        // leer lo que mando el cliente (por ahora no se usa) 
        n = read(client, buf, sizeof(buf));
        if (n > 0) {
            send(client, response, strlen(response), 0);
        }

        close(client);
    }
    return 0;
}