# MiniHTTPd

Servidor HTTP/1.1 básico en C para Linux, sin bibliotecas HTTP externas.
Sirve archivos estáticos y atiende varios clientes a la vez con `epoll`.

## Qué hace

- Atiende solo el método `GET` (cualquier otro método responde 405).
- Lee los encabezados `Host`, `Connection` y `User-Agent`; los demás se ignoran.
- Usa conexiones persistentes: la conexión se mantiene abierta, salvo que el cliente mande `Connection: close`.
- Atiende varios clientes con `epoll` y sockets no bloqueantes. Si un cliente lento no puede recibir todo de una vez, el servidor guarda cuánto ha enviado y sigue cuando `epoll` avisa (`EPOLLOUT`), sin bloquear a los demás.
- Devuelve el tipo MIME según la extensión del archivo.

## Códigos de estado

| Código | Cuándo |
|--------|--------|
| 200 OK | El archivo existe y se envía |
| 400 Bad Request | Solicitud mal formada, versión distinta de HTTP/1.1 o solicitud demasiado grande |
| 403 Forbidden | Sin permiso o ruta fuera de la raíz |
| 404 Not Found | El archivo no existe |
| 405 Method Not Allowed | Método distinto de GET |
| 500 Internal Server Error | Error interno |

## Tipos MIME

`.html`, `.css`, `.js`, `.png` y `.jpg`. Cualquier otra extensión se envía como `application/octet-stream`.

## Seguridad

- **Directory traversal:** la ruta pedida se resuelve con `realpath()` y se verifica que quede dentro de la raíz (`/../../etc/passwd` no funciona).
- **Buffer overflow:** no se usa `strcpy` ni `sprintf`; se usan `snprintf` y copias con el tamaño comprobado.
- **Tamaños:** la solicitud se limita a 8192 bytes, la línea de solicitud a 2048 y la URI a 1024. Si se pasan, responde 400.

## Archivos

- `src/main.c`: programa principal y bucle de `epoll`
- `src/server.c`: socket, tabla de clientes y atención de cada cliente
- `src/http.c`: análisis de la solicitud, encabezados y respuestas
- `src/files.c`: raíz de documentos, resolución de rutas y apertura de archivos
- `src/mime.c`: tabla de tipos MIME
- `include/`: archivos de cabecera (`server.h`, `http.h`, `files.h`, `mime.h`)
- `www/`: contenido de ejemplo (`index.html`, `style.css`, `image.png`)
- `Makefile`: compilación

## Compilar

    make

## Ejecutar

    ./minihttpd              # puerto 8080, raíz "www"
    ./minihttpd 8080 www     # puerto y raíz explícitos

## Probar

    curl -i http://localhost:8080/index.html          # 200
    curl -i http://localhost:8080/noexiste.html       # 404
    curl -i -X POST http://localhost:8080/index.html  # 405
    curl -i --path-as-is http://localhost:8080/../../etc/passwd   # no sale de la raíz
    printf "HOLA\r\n\r\n" | nc localhost 8080         # 400

Conexión persistente (dos archivos por la misma conexión):

    curl -v http://localhost:8080/index.html http://localhost:8080/style.css


## Limpiar

    make clean