#ifndef FILES_H
#define FILES_H

//tamaño max ruta
#define MAX_PATH 4096

//Fijar raiz de documentos y comprubar que sea un directorio
int init_root(const char *path);

//devolverla ruta de la raiz
const char *get_root(void);

//resolver la URI pedida dentro de la raiz de documentos y devolver la ruta absoluta
int resolve_path(const char *uri, char *out, int size);

//abrir archivo para leerlo y obtener su tamaño
int open_file(const char *path, int *fd, long *size);

#endif