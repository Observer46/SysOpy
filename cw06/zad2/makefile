CC = gcc -Wall

all:
	make server
	make client

client: posix_client.c posix_chat_settings.h
	$(CC) posix_client.c -o client -lrt

server: posix_server.c posix_chat_settings.h
	$(CC) posix_server.c -o server -lrt

clean:
	rm -f server client
