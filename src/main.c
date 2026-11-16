#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/epoll.h>
#include "server.h"
#include "http.h"
#include "files.h"
#include "mime.h"


#define PORT 8080
#define MAX_EVENTS 10 



int main(int argc, char *argv[])
{
    int port = PORT;
    int server, client, ep, n, i;
    const char *root_dir = "www";
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    //char buff[TAM_BUFF + 1];
    //struct  request_line request;
    //char response[1024];
    //int code, len;
    //struct header headers;
    //char file_path[MAX_PATH];
    //int file_fd;
    //long file_size;

    if (argc > 1)
        port = atoi(argv[1]);
    if(argc > 2){
        root_dir = argv[2];
    }


    //fijar raiz del servidor
    if(init_root(root_dir) < 0){
        return 1;
    }

    server = create_server(port);
    if (server < 0)
        return 1;
    
    //instanciar epoll
    ep = epoll_create1(0);
    if(ep < 0){
        perror("epoll_create1");
        return 1;
    }
    
    //registrar el socket ecucha y avisar cuando haya eventos
    ev.events = EPOLLIN;
    ev.data.fd = server;
    if(epoll_ctl(ep, EPOLL_CTL_ADD, server, &ev) < 0){
        perror("epoll_ctl");
        return 1;
    }

    printf("Servidor escuchando en el puerto %d\n", port);
    printf("Directorio raiz: %s\n", get_root());

    //bucle principal
    while(1){
        //dormir hasta que haya eventos
        n = epoll_wait(ep, events, MAX_EVENTS, -1);
        if(n < 0){
            if(errno == EINTR){
                continue;
            }
            perror("epoll_wait");
            break;
        }

        for(i=0; i<n; i++){
            if(events[i].data.fd != server){
                continue;
            }

            //aceptar los pendientes
            while(1){
                client = accept(server, NULL, NULL);
                if (client < 0){
                    if (errno != EINTR && errno != EWOULDBLOCK){
                        perror("accept");
                    }
                    break;
                }
                handle_client(client);
            }
        }
    }
    close(ep);
    close(server);


    /*
    printf("Servidor escuchando en el puerto %d\n", port);
    printf("Directorio raiz: %s\n", get_root());
    while (1) {
        // esperar a que llegue un cliente 
        client = accept(server, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR || errno == EWOULDBLOCK){
                usleep(1000);
                continue;
            }
            perror("accept");
            continue;
        }

        file_fd = -1;
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
                code = check_request_line(&request);    
                if(code != 200){
                    printf("Solicitud invalida\n");
                } else if(parse_headers(buff, &headers) == 0){
                    printf("Host : %s\n", headers.host);
                    printf("Connection : %s\n", headers.connection);
                    printf("User-Agent : %s\n", headers.user_agent);
                    code = resolve_path(request.uri, file_path, sizeof(file_path));
                    if (code == 200){
                        printf("Archivo a servir: %s (%s)\n", file_path, get_mime_type(file_path));
                        //abrir archivo y tomar su tamaño
                        code = open_file(file_path, &file_fd, &file_size);
                        if(code != 200){
                            printf("Error al abrir archivo: %d\n", code);
                        }
                    }else{
                        printf("Ruta rechazada con %d\n", code);
                    }
                } else {
                    printf("Error al parsear encabezados\n");
                    code = 400;
                }
            }else {
                printf("error linea de solicitud\n");
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

        if(code == 200 && file_fd >= 0){
            //encabezado con tipo y tamaño
            len = build_file_header(get_mime_type(file_path), file_size, response, sizeof(response));
            if(len > 0 && write_all(client, response, len) == 0){
                send_file(client, file_fd);
            }
            close(file_fd);
        }else {
            //respuesta de error con el codigo que corresponde
            len = build_response(code, response, sizeof(response));
            if(len > 0){
                write_all(client, response, len);
            }    
        }
        close(client);
    }*/
    return 0;
}