minihttpd: src/main.c src/server.c src/http.c
	gcc -Wall -Wextra -Iinclude src/main.c src/server.c src/http.c -o minihttpd

clean:
	rm -f minihttpd