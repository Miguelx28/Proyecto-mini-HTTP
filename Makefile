minihttpd: src/main.c src/server.c src/http.c src/mime.c src/files.c
	gcc -Wall -Wextra -Iinclude src/main.c src/server.c src/http.c src/mime.c src/files.c -o minihttpd

clean:
	rm -f minihttpd