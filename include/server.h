# ifndef SERVER_H
# define SERVER_H

// Crear srvidor y dejar escuchando, devuelve el  descriptor o -1 si hubo error
int create_server(int port);

//poner descriptor en modo no bloqueante
int set_nonblocking(int fd);

//atender un cliente ya aceptao
void handle_client(int client);
#endif