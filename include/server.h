# ifndef SERVER_H
# define SERVER_H

#define TAM_BUFF 8192
#define MAX_CLIENTS 64

// En que pinto esta cada cliente
enum client_state {
    CLIENT_FREE,
    CLIENT_READING,
    CLIENT_WRITING,
    CLIENT_CLOSING
};

// Estructura que contiene lo que el servidor necesita de un cliente
struct client {
    int fd;
    enum client_state state;
    char in[TAM_BUFF+1];
    int in_len;
    char out[1024];
    int out_len;
    int out_sent;
    int file_fd;
    long file_size;
    long file_sent;
    const char *mime;
    int keep_alive;
};


//Registrar un cliente nuevo a la tabla
struct client *client_add(int fd);

//liberar al cliente de la tabla
void client_remove(struct client *client);

//contador de clientes
int client_count(void);

// Crear srvidor y dejar escuchando, devuelve el  descriptor o -1 si hubo error
int create_server(int port);

//poner descriptor en modo no bloqueante
int set_nonblocking(int fd);

//poner descriptor en modo bloqueante

int set_blocking(int fd);
//atender un cliente ya aceptao
int handle_client(struct client *c);

//dejar al cliente listo para otra solicitud
void client_reset(struct client *c);
#endif