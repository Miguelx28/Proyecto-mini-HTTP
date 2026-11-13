# MiniHTTPd
Servidor HTTP1.1 básico en C, sin librerias externas.

El servidor escucha 



El servidor escucha en un puerto TCP y acepta coneciones de una en una.
Por cada cliente:

1. Lee la solicitud completa. Como TCP entrega un flujo de bytes, va acumulando datos en un buffer de 8192 bytes hasta encontrar '\r\n\r\n', como fin de encabezado.
2. Separa la primera línea en método, URI y version.
3. Decide el coodigo dde estado:
    - 200 OK si la linea esta bine formada
    - 400 Bad Request si esta mal formada o demacidad grande
4. Arma la respuesta HTTP completa.
5. Cierra la conecxion.

La función que arma la respuesta también sabe generar 403, 404, 405 y 500,
que se usarán cuando el servidor valide más cosas.
 
Las copias de texto se hacen comprobando el tamaño antes de copiar, sin
`strcpy` ni `sprintf`, para evitar desbordamientos de buffer.

## Archivos
- `src/main.c`: programa principal y bucle de `accept()`
- `src/server.c`: crea el socket (`socket`, `bind`, `listen`)
- `src/http.c`: analiza la solicitud
- `include/server.h` y `include/http.h`: declaraciones
- `Makefile`: compilación

## Compilar
    make

## Ejecutar
    ./minihttpd 8080

El puerto es opcional se ejecuta(8080 por defeto). Para limpiar -> `make clean`.
 
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