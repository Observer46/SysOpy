CC = gcc -Wall

all:
	make server
	make client

client: systemV_client.c
	$(CC) systemV_client.c -o client

server: systemV_server.c
	$(CC) systemV_server.c -o server

clean:
	rm -f server client
