#ifndef HHTP_h
#define HHTP_h

#define MAX_URI 1024

//tres partes  de la linea de la peticion
struct request_line {
    char method[16];
    char uri[MAX_URI];
    char version[16];
};

//separar la primera linea y llenar la estructura 
int parse_request_line(const char *line, struct request_line *request);

#endif