#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "server.h"
#include "http.h"
#include "files.h"
#include "mime.h"

#define TAM_BUFF 8192

int set_nonblocking(int fd){
    int flags;

    //leer las banderas actuales y asignar O_NONBLOCK
    flags = fcntl(fd, F_GETFL, 0);
    if(flags < 0){
        return -1;
    }
    if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0){
        return -1;
    }
    return 0;
}


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

    //no bloquear el socket
    if(set_nonblocking(fd) < 0){
        perror("set_nonblocking");
        close(fd);
        return -1;
    }
    return fd;
}  

//escribir los "len" bytes completos
static int write_all(int fd, const char *data, int len){
    int sent = 0;
    int n;

    while(sent < len){
        n = write(fd, data + sent, len - sent);
        if(n <= 0){
            return -1;
        }
        sent += n;
    }
    return 0;
}

//leer el archivo por bloques y enviarlo al cliente
static void send_file(int client, int file_fd){
    char chunk[TAM_BUFF];
    int n;
    while((n = read(file_fd, chunk, sizeof(chunk))) > 0){
        if(write_all(client, chunk, n) < 0){
            break;
        }
    }
}

void handle_client(int client){
    char buff[TAM_BUFF + 1];
    char response[1024];
    struct request_line request;
    struct header headers;
    char file_path[MAX_PATH];
    int file_fd = -1;
    long file_size = 0;
    int n, total, complete, code, len;

    //leer la solicitud completa
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
        return;
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
}    

