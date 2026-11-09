minihttpd: src/main.c src/server.c
	gcc -Wall -Iinclude src/main.c src/server.c -o minihttpd

clean:
	rm -f minihttpd