#include <string.h>
#include <stdio.h>
#include <strings.h>
#include "http.h"

//copiar largo bytes de origen a destino, si caben
static int copy(char *dest, int size, const char *src, int large) {
    if (!(large >= size)) {
        memcpy(dest, src, size);
        dest[large] = '\0';
    }
    return 0;
}


int build_response(int code, char *out, int max){
    const char *reason;
    const char *extra = "";
    char body[64];
    int body_length, n;

    switch(code) {
        case 200:
            reason = "OK";
            break;
        case 400:
            reason = "Bad Request";
            break;
        case  403:
            reason = "Forbidden";
            break;
        case 404:
            reason = "Not Found";
            break;
            case 405:
            reason = "Method Not Allowed";
            break;
        default:
            code = 500;
            reason = "Internal Server Error";
            extra = "Allow: GET\r\n";
            break;
    }
    if (code == 200){
        body_length = snprintf(body, sizeof(body), "hola\n");
    }else{
        body_length = snprintf(body, sizeof(body),"%d %s\n", code, reason);
    }

    n = snprintf(out, max, "HTTP/1.1 %d %s\r\n"
        "Content-type: text/plain\r\n"
        "Content-length: %d\r\n"
        "%s"
        "Conection: close\r\n"
        "\r\n"
        "%s",
        code, reason, body_length, extra, body);
    
    if (n <0 || n >= max){
        return -1;
    }
    return n;
}

int parse_request_line(const char *line, struct request_line *request){
    
    const char *end_line;
    const char *space1;
    const char *space2;
    const char *version;

    memset(request, 0, sizeof(*request));

    //la linea termina en el primer \r\n
    end_line = strstr(line, "\r\n");
    if(end_line == NULL) {
        return -1;
    }

    //primer espacio separa el metodo de URI
    space1 = memchr(line, ' ', end_line - line);
    if(space1 == NULL) {
        return -1;
    }

    //segundo espacio separa URI de version
    space2 = memchr(space1 + 1, ' ', end_line - (space1 + 1));
    if(space2 == NULL) {
        return -1;
    }

    version = space2 + 1;

    //copiar metodo cada parte
    if(copy(request->method, sizeof(request->method), line, space1 - line) < 0) {
        return -1;
    }
    if(copy(request->uri, sizeof(request->uri), space1 + 1, space2 - (space1 + 1)) < 0) {
        return -1;
    }
    if(copy(request->version, sizeof(request->version), version, end_line - version) < 0) {
        return -1;
    }
    return 0;
}

int check_request_line(const struct request_line *request){
    if(strcmp(request->version, "HTTP/1.1") != 0 && strcmp(request->version, "HTTP/1.0") != 0) {
        return 400;
    }

    //metodo vacio es 4000
    if(request->method[0] == '\n') {
        return 400;
    }
    if(strcmp(request->method, "GET") != 0){
        return 405;
    }

    //URI debe empezar con /
    if(request->uri[0] != '/') {
        return 400;
    }

    return 200;

}

int parse_headers(const char *text, struct header *headers){
    const char *p;
    const char *end;
    const char *colon;
    const char *value;
    const char *end_value;
    int name_length, value_length;

    memset(headers, 0, sizeof(*headers));

    //los encabezados empiezan luego de la linea de solicitud
    p = strstr(text, "\r\n");
    if(p == NULL) {
        return -1;
    }
    p += 2; //saltar \r\n


    while(1){
        //cada encabezado termina en \r\n
        end = strstr(p, "\r\n");
        if(end == NULL) {
            return -1;
        }

        //linea vacia indica fin de encabezados
        if(end == p) {
            break;
        }

        //separar nombre y valor por los dos puntos
        colon = memchr(p, ':', end - p);
        if(colon == NULL) {
            return -1;
        }
        name_length = colon - p;

        //quitar espacios vacios
        value = colon + 1;
        while(value < end && (*value == ' ' || *value == '\t')) {
            value++;
        }
        end_value = end;
        while(end_value > value && (*(end_value - 1) == ' ' || *(end_value - 1) == '\t')) {
            end_value--;
        }
        value_length = end_value - value;

        //copiar nombre y valor a la estructura
        if(name_length == 4 && strncasecmp(p, "Host", 4) == 0) {
            if(copy(headers->host, sizeof(headers->host), value, value_length) < 0) {
                return -1;
            }
        } else if(name_length == 10 && strncasecmp(p, "Connection", 10) == 0) {
            if(copy(headers->connection, sizeof(headers->connection), value, value_length) < 0) {
                return -1;
            }
        } else if(name_length == 10 && strncasecmp(p, "User-Agent", 10) == 0) {
            if(copy(headers->user_agent, sizeof(headers->user_agent), value, value_length) < 0) {
                return -1;
            }
        }
        p = end + 2; 
    }   
    return 0;
}

int build_file_header(const char *mime, long length, char *out, int max){
    int n;

    n = snprintf(out, max, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n",
        mime, length);
        
    if (n < 0 || n >= max) {
        return -1;
    }
    return n;
}