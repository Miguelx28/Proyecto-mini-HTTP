#include <string.h>
#include <stdio.h>
#include <strings.h>
#include "http.h"

int parse_request(const char *text, struct request *request){
    char line[MAX_LINE +1];
    char extra[2];
    const char *end;
    int len;

    end = strstr(text, "\r\n");
    if(end == NULL || end - text >= MAX_LINE) {
        return 400;
    }
    len = end - text;
    memcpy(line, text, len);
    line[len] = '\0';
    //verificar que la linea de solicitud sea validaue tenga 3 partes
    if(sscanf(line, "%15s %1024s %15s %1s", request->method, request->uri, request->version, extra) != 3){
        return 400;
    }

    if(strcmp(request->version, "HTTP/1.1")){
        return 400;
    }

    if (strcmp(request->method, "GET") != 0) {
        return 405;
    }

    if (request->uri[0] != '/') {
        return 400;
    }
    return 200;
}

int get_header(const char *text, const char *name, char *out, int size){
    const char *p;
    const char *end;
    const char *value;
    int name_len = strlen(name);
    int len;

    //los encabezados empiezan luego de la linea de solicitud
    p = strstr(text, "\r\n");
    if(p == NULL) {
        return -1;
    }
    p += 2;

    //recorer linea por linea
    while((end = strstr(p, "\r\n")) != NULL && end != p) {
        if(strncasecmp(p, name, name_len) == 0 && p[name_len] == ':') {
            value = p + name_len + 1;
            while(value < end && (*value == ' ' || *value == '\t')) {
                value++;
            }
            len = end - value;
            if(len >= size) {
                return -1;
            }
            memcpy(out, value, len);
            out[len] = '\0';
            return 0;
        }
        p = end + 2;
    }
    return -1;
}

int build_response(int code, int keep_alive, char *out, int max){
    const char *reason;
    const char *extra = "";
    char body[64];
    int body_length, n;
    const char *conn;

    if (keep_alive) {
        conn = "keep-alive";
    } else {
        conn = "close";
    }

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
            extra = "Allow: GET\r\n";
            break;
        default:
            code = 500;
            reason = "Internal Server Error";
            break;
    }

    if (code == 200){
        body_length = snprintf(body, sizeof(body), "hola\n");
    }else{
        body_length = snprintf(body, sizeof(body),"%d %s\n", code, reason);
    }

    n = snprintf(out, max, "HTTP/1.1 %d %s\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "%s"
        "Connection: %s\r\n"
        "\r\n"
        "%s",
        code, reason, body_length, extra, conn, body);
    
    if (n <0 || n >= max){
        return -1;
    }
    return n;
}

int build_file_header(const char *mime, long length, int keep_alive, char *out, int max){
    int n;
    const char *conn;
    if (keep_alive) {
        conn = "keep-alive";
    } else {
        conn = "close";
    }

    n = snprintf(out, max, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: %s\r\n"
        "\r\n",
        mime, length, conn);
        
    if (n < 0 || n >= max) {
        return -1;
    }
    return n;
}