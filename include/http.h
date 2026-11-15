#ifndef HHTP_h
#define HHTP_h

#define MAX_URI 1024

//tres partes  de la linea de la peticion
struct request_line {
    char method[16];
    char uri[MAX_URI];
    char version[16];
};

//encabezados
struct header {
    char host[256];
    char connection[32];
    char user_agent[512];
};

//separar la primera linea y llenar la estructura 
int parse_request_line(const char *line, struct request_line *request);

//revisar que la solicitud sea valida
int check_request_line(const struct request_line *request);

//Armar respuesta HHTP completa
int build_response(int code, char *out, int max);

//recorer encabezados
int parse_headers(const char *text, struct header *headers);

#endif