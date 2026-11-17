#ifndef HHTP_H
#define HHTP_H

#define MAX_URI 1024
#define MAX_LINE 2048

//tres partes  de la linea de la peticion
struct request {
    char method[16];
    char uri[MAX_URI+1];
    char version[16];
};

//Analixar y validar la linea de solicitud
int parse_request(const char *text, struct request *request);

//buscar el encabezado
int get_header(const char *text, const char *name, char *out, int size);

//Armar respuesta HHTP completa
int build_response(int code, int keep_alive, char *out, int max);

//arma solo el encabezado de una respuesta que lleva un archivo
int build_file_header(const char *mime, long length,int keep_alive, char *out, int max);

#endif