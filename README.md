# MiniHTTPd
Servidor HTTP1.1 básico en C, sin librerias externas.

El servidor escucha 



El servidor escucha en un puerto TCP y acepta coneciones de una en una.
Por cada cliente:

1. Lee la solicitud completa. Como TCP entrega un flujo de bytes, va acumulando datos en un buffer de 8192 bytes hasta encontrar '\r\n\r\n', como fin de encabezado.
2. Separa la primera línea en método, URI y version.
3. Imprime esas tres partes en la terminal.
4. Responde con el texto "hola" y cierra la conecxión.

Si la linea de solicitud esta mal formada(sin espacion, version o con una parte muy larga), lo indica en la terminal.

Las copias de texto se hacen comprobando el tamaño antes de copiar, sin `strpy` ni `strintf`, para evitar desbordamiento del buffer.

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
    curl http://localhost:8080/index.html

Respuesta: `hola`

En la terminal del servidor aparece:

    metodo : [GET]
    uri    : [/index.html]
    version: [HTTP/1.1]

## Limpiar

    make clean