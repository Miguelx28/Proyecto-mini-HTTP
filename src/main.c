#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"
#include "http.h"


#define PORT 8080
#define TAM_BUFF 8192 



int main(int argc, char *argv[])
{
    int port = PORT;
    int server, client;
    char buff[TAM_BUFF + 1];
    int n, total, complete;
    struct  request_line request;
    char response[1024];
    int code, len;

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
        //leer hasta encontrar fin encabezados
        total = 0;
        complete = 0;
        while(total < TAM_BUFF){
            n =  read(client, buff+total,TAM_BUFF-total);
            if(n <= 0){
                break;
            }
            total += n;
            buff[total] = '\0';
            if(strstr(buff, "\r\n\r\n") != NULL){
                complete = 1;
                break;
            }
        }
        if(complete){
            //parsear la primera linea de la peticion
            if(parse_request_line(buff, &request) ==0){
                printf("Metodo: %s\n", request.method);
                printf("URI: %s\n", request.uri);
                printf("Version: %s\n", request.version); 
                code = 200;
            }else {
                printf("error linea dde solicitud\n");
                code = 400;
            }
            //enviar respuesta
            //write(client, response, strlen(response));
        }else if(total >= TAM_BUFF){
            printf("Solicitud demasiado grande\n");
            code = 400;
        }else{
            //cliente cerrado sin terminar la solicitud
            printf("Cliente cerrado sin terminar la solicitud ");
            close(client);
            continue;
        }

        //armar respuesta
        len = build_response(code, response, sizeof(response));
        printf("code=%d len=%d\n", code, len);
        if(len > 0){
            write(client, response, len);
        }
        close(client);
    }
    return 0;
}