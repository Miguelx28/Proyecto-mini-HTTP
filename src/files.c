#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "files.h"

//ruta real de los documentos
static char root[MAX_PATH];

int init_root(const char *path){
    struct stat info;

    //convertir la ruta a absoluta
    if(realpath(path, root) ==NULL){
        perror("Error al obtener la ruta absoluta");
        return -1;
    }

    //comprobar que es un directorio
    if(stat(root, &info) < 0 || !S_ISDIR(info.st_mode)){
        fprintf(stderr, "Error: %s no es un directorio\n", root);
        return -1;
    }
    return 0;
}

const char *get_root(void){
    return root;
}

int resolve_path(const char *uri, char *out, int size){
    char path[MAX_PATH];
    char candidate[MAX_PATH];
    char resolved[MAX_PATH];
    struct stat info;
    int len, n, root_len;

    len = strcspn(uri, "?");
    if(len >= (int)sizeof(path)){
        return 400;
    }
    memcpy(path, uri, len);
    path[len] = '\0';

    //verificar si termina con '/' y agregar "index.html" si es así
    if(len > 0 && path[len - 1] == '/'){
        if(len +11 > (int)sizeof(path)){
            return 400;
        }
        memcpy(path + len, "index.html", 11);
    }

    //unir la raiz con la ruta pedida
    n = snprintf(candidate, sizeof(candidate), "%s/%s", root, path);
    if(n < 0 || n >= (int)sizeof(candidate)){
        return 400;
    }

    //resolver la ruta absoluta
    if(realpath(candidate, resolved) == NULL){
        if(errno == ENOENT || errno == ENOTDIR || errno == ELOOP){
            return 404;
        }
        if (errno == EACCES){
            return 403;
        }
        if (errno == ENAMETOOLONG){
            return 400;
        }
        return 500;
    }


    //verificar que la ruta resuelta esté dentro de la raíz
    root_len = strlen(root);
    if(strncmp(resolved, root, root_len) != 0){
        return 403;
    }
    if(resolved[root_len] != '/' && resolved[root_len] != '\0'){
        return 403;
    }

    //verificar que el archivo exista y sea un archivo normal
    if(stat(resolved, &info) < 0){
        return 500;
    }
    if(!S_ISREG(info.st_mode)){
        return 403;
    }

    //copiar la ruta resuelta al buffer de salida
    if((int)strlen(resolved) >= size){
        return 500;
    }
    memcpy(out, resolved, strlen(resolved) + 1);

    return 200;

}

int open_file(const char *path, int *fd, long *size){
    struct stat info;
    int f;

    f = open(path, O_RDONLY);
    if(f < 0){
        if(errno == EACCES){
            return 403;
        }
        if(errno == ENOENT){
            return 404;
        }
        return 500;
    }

    if(fstat(f, &info) < 0 || !S_ISREG(info.st_mode)){
        close(f);
        return 500;
    }

    *fd = f;
    *size = info.st_size;
    return 200;
}