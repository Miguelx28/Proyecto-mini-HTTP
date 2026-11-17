minihttpd: src/main.c src/server.c src/http.c src/mime.c src/files.c include/server.h include/http.h include/mime.h include/files.h
	gcc -Wall -Wextra -Iinclude src/main.c src/server.c src/http.c src/mime.c src/files.c -o minihttpd

clean:
	rm -f minihttpd

.PHONY: clean