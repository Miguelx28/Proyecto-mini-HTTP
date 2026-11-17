#include <string.h>
#include <strings.h>
#include "mime.h"

//tabla de extenciones con su tipo de minme
struct mime_entry{
    const char *ext;
    const char *type;
};

static const struct mime_entry table[] = {
    { ".html", "text/html" },
    { ".css", "text/css" },
    { ".js", "application/javascript" },
    { ".png", "image/png" },
    { ".jpg", "image/jpeg" }
};

const char *get_mime_type(const char *path) {
    const char *slash;
    const char *name;
    const char *dot;
    int i;
    int count = sizeof(table) / sizeof(table[0]);

    //nombre de archivo
    slash = strrchr(path, '/');
    if (slash == NULL) {
        name = path;
    } else {
        name = slash + 1 ;
    }
    
    //la extension del archivo
    dot = strrchr(name, '.');
    if (dot == NULL) {
        return "application/octet-stream";
    }

    //buscar en la tabla
    for(i = 0; i < count; i++) {
        if(strcasecmp(dot, table[i].ext) == 0){
            return table[i].type;
        }   
    }
    return "application/octet-stream";
} 
