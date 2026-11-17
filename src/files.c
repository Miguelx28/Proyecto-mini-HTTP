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
    //convertir la ruta a absoluta
    if(realpath(path, root) ==NULL){
        perror("Error al obtener la ruta de la raíz");
        return -1;
    }
    return 0;
}


int resolve_path(const char *uri, char *out, int size){
    char candidate[MAX_PATH];
    char resolved[MAX_PATH];
    const char *index = ""; 
    int n, root_len;

    //verificar si la ruta termina con '/'
    if(uri[strlen(uri) - 1] == '/'){
        index = "index.html";
    }
    n = snprintf(candidate, sizeof(candidate), "%s%s%s", root, uri, index);
    if (n >= (int)sizeof(candidate)){
        return 404;
    }


    //resolver la ruta absoluta
    if(realpath(candidate, resolved) == NULL){
        if(errno == EACCES){
            return 403;
        }
        if(errno == ENOMEM || errno == EIO){
            return 500;
        }
        return 404;
    }
    

    //verificar que la ruta resuelta esté dentro de la raíz
    root_len = strlen(root);
    if(strncmp(resolved, root, root_len) != 0){
        return 403;
    }
    if(resolved[root_len] != '/' && resolved[root_len] != '\0'){
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