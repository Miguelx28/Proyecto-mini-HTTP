#include <string.h>
#include <stdio.h>
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
            reason = "Internal Server Error";
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