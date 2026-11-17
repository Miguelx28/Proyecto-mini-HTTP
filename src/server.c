#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <netinet/in.h>
#include <strings.h>
#include "server.h"
#include "http.h"
#include "mime.h"
#include "files.h"

#define TAM_BUFF 8192

static struct client clients[MAX_CLIENTS];

struct client *client_add(int fd){
    struct client *c;
    
    if(fd < 0 || fd >= MAX_CLIENTS){
        return NULL;
    }

    c = &clients[fd];
    memset(c, 0, sizeof(struct client));
    c->fd = fd;
    c-> file_fd = -1;
    c->state = CLIENT_READING;
    return c;
}

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

int set_blocking(int fd){
    int flags;

    //leer las banderas actuales y quitar O_NONBLOCK
    flags = fcntl(fd, F_GETFL, 0);
    if(flags < 0){
        return -1;
    }
    if(fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0){
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

void client_remove(struct client *c){
    if(c->state == CLIENT_FREE){
        return;
    }
    if(c->file_fd >= 0){
        close(c->file_fd);
    }
    close(c->fd);
    c->file_fd = -1;
    c->fd = -1;
    c->state = CLIENT_FREE;
}

void client_reset(struct client *c){
    if(c->file_fd >= 0){
        close(c->file_fd);
    }
    c->file_fd = -1;
    c->in_len = 0;
    c->in[0] = '\0';
    c->out_len = 0;
    c->out_sent = 0;
    c->file_size = 0;
    c->file_sent = 0;
    c->keep_alive = 0;
    c->state = CLIENT_READING;
}

//validar la solicitud y buscar el archivo
static int process_request(struct client *c){
    struct request request;
    char path[MAX_PATH];
    char host[256];
    char agent[512];
    int code;
    char conn[64];

    code = parse_request(c->in, &request);
    if(code != 200){
        return code;
    }

    //encabezados
    host[0] = '\0';
    agent[0] = '\0';
    get_header(c->in, "Host", host, sizeof(host));
    get_header(c->in, "User-Agent", agent, sizeof(agent));
    printf("Host: %s\n", host);
    printf("User-Agent: %s\n", agent);

    //
    c->keep_alive = 1;
    if(get_header(c->in, "Connection", conn, sizeof(conn)) == 0 && strcasecmp(conn, "close") ==0){
        c->keep_alive = 0;
    }

    code = resolve_path(request.uri, path, sizeof(path));
    if(code != 200){
        return code;
    }

    c->mime = get_mime_type(path);
    return open_file(path, &c->file_fd, &c->file_size);
}

int handle_client(struct client *c){    
    off_t offset = 0;
    ssize_t n;
    int code;

    //leer la solicitud del cliente
    if (c->state == CLIENT_READING){               
            while(c->in_len < TAM_BUFF && strstr(c->in, "\r\n\r\n") == NULL){
                n = read(c->fd, c->in + c->in_len, TAM_BUFF - c->in_len);
                if(n > 0){
                    c->in_len += n;
                    c->in[c->in_len] = '\0';
                    continue;
                }
                if(n < 0 && errno == EINTR){
                    continue;
                }
                if(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
                    return 0;
                }
                return -1;
            }
        
        //solicitud demaciada grande
        if(strstr(c->in, "\r\n\r\n") == NULL){
            code = 400;
        }else{
            code = process_request(c);
        }
        if(code == 200){
            c->out_len = build_file_header(c->mime, c->file_size, c->keep_alive, c->out, sizeof(c->out));
        }else{
            c->out_len = build_response(code, c->keep_alive, c->out, sizeof(c->out));
        }

        if (c->out_len <= 0){
            return -1;
        }
        c->state = CLIENT_WRITING;
    }

    //enviar encavezado
    while(c->out_sent < c->out_len){
        n = write(c->fd, c->out + c->out_sent, c->out_len - c->out_sent);
        if(n > 0){
            c->out_sent += n;
            continue;
        }
        if(n < 0 && errno == EINTR){
            continue;
        }
        if(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
            return 2;
        }
        return -1;
    }

    //contenido del archivo
    if(c->file_fd >= 0) {
        while(c->file_sent < c->file_size){
            offset = c->file_sent;
            n = sendfile(c->fd, c->file_fd, &offset, c->file_size - c->file_sent);
            if(n > 0){
                c->file_sent += n;
                continue;
            }
            if(n < 0 && errno == EINTR){
                continue;
            }
            if(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
                return 2;
            }
            return -1;
        }
    }
    return 1;
}    

