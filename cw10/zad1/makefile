# Makefile
CC = gcc -Wall

all:
	make client
	make server

client: client.c utils.h utils.c sendData.h
	$(CC) client.c utils.c -o client

server: server.c utils.h utils.c sendData.h
	$(CC) server.c utils.c -o server -lpthread
	
clean:
	rm -f client server