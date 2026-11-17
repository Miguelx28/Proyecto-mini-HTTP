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
    int r;

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
    ev.data.ptr = NULL; 
    if(epoll_ctl(ep, EPOLL_CTL_ADD, server, &ev) < 0){
        perror("epoll_ctl");
        close(ep);
        close(server);
        return 1;
    }

    printf("Servidor escuchando en el puerto %d\n", port);

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
            //nueva conecxion
            if(events[i].data.ptr == NULL){
                //aceptar los pendientes
                while(1){
                    client = accept(server, NULL, NULL);
                    if (client < 0){
                        if (errno == EINTR){
                            continue;
                        }  
                        if(errno == EAGAIN || errno == EWOULDBLOCK){
                            break;
                        }
                        perror("accept");
                        break;
                    }
                    //el cliente debe ser no bloqueante
                    if(set_nonblocking(client) < 0){
                        perror("set_nonblocking");
                        close(client);
                        break;
                    }
                    //registrar cliente
                    struct client *c = client_add(client);
                    if(c == NULL){
                        printf("No se pudo agregar cliente\n");
                        close(client);
                        continue;
                    }
                    //Registrar cliente en epoll
                    ev.events = EPOLLIN;
                    ev.data.ptr = c;
                    if(epoll_ctl(ep, EPOLL_CTL_ADD, client, &ev) < 0){
                        perror("epoll_ctl");
                        client_remove(c);
                    }
                }
            }else{
                //evento de cliente
                struct client *c = events[i].data.ptr;
                if(events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)){
                    epoll_ctl(ep, EPOLL_CTL_DEL, c->fd, NULL);
                    client_remove(c);
                    continue;
                }
                r = handle_client(c);
                if(r == 0){
                    continue;
                }
                if(r ==2){
                    ev.events = EPOLLOUT;
                    ev.data.ptr = c;
                    epoll_ctl(ep, EPOLL_CTL_MOD, c->fd, &ev);
                    continue;
                }
                if(r == 1 && c->keep_alive){
                    client_reset(c);
                    ev.events = EPOLLIN;
                    ev.data.ptr = c;
                    epoll_ctl(ep, EPOLL_CTL_MOD, c->fd, &ev);
                    continue;
                }
                epoll_ctl(ep, EPOLL_CTL_DEL, c->fd, NULL);
                client_remove(c);
            }
        }
    }
    close(ep);
    close(server);
    return 0;
}