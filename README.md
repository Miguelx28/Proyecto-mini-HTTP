# MiniHTTPd
Servidor HTTP1.1 básico en C, sin librerias externas.

El servidor escucha 



El servidor escucha en un puerto TCP y acepta coneciones de una en una.
Por cada cliente:

1. Lee la solicitud completa. Como TCP entrega un flujo de bytes, va acumulando datos en un buffer de 8192 bytes hasta encontrar '\r\n\r\n', como fin de encabezado.
2. Separa la primera línea en método, URI y version.
3. Valida la linea y decide el código de estado:
    * 200 OK
    * 405 Method Not Allowed
    * 400 Bad Request
4. Recorre los en encabezados y guarda Host, Connection y User, los demas encabezados se ignoran.
5. Busca el archivo pedido dentro de la raiz de documentos
6. Arma la respuesta HTTP completa
7. Cierra la conección.
 
Las copias de texto se hacen comprobando el tamaño antes de copiar, sin
`strcpy` ni `sprintf`, para evitar desbordamientos de buffer.

## Archivos
- `src/main.c`: programa principal y bucle de `accept()`
- `src/server.c`: crea el socket (`socket`, `bind`, `listen`)
- `src/files.c`: fija y consulta la raiz de documentos
- `src/mime.c`: tabla de extensiones y tipos MIME 
- `src/mime.c`: tabla de  extensiones y tipos MIME
- `src/http.c`: separa la línea de solicitus, lee los encabezados y arma las respuestas
- `include/server.h` y `include/http.h` e `include/mime.h`, e `include/files.h` e ` include/mime.h`: declaraciones
- `Makefile`: compilación

## Compilar
    make

## Ejecutar
    ./minihttpd 8080
    ./minihttpd 8080 www

El puerto es opcional y la raiz tambien se ejecuta(8080, www) por defecto. 
 
## Probar
 
    curl -v http://localhost:8080/index.html
 
Respuesta:
 
    HTTP/1.1 200 OK
    Content-Type: text/plain
    Content-Length: 5
    Connection: close
 
    hola
 


Solicitud mal formada:
 
    printf "HOLA\r\n\r\n" | nc localhost 8080
 
Respuesta: `HTTP/1.1 400 Bad Request`
 
## Limpiar
 
    make clean