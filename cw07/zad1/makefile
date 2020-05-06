CC = gcc -Wall

#all:
#	make server
#	make client

main: main.c utils.c utils.h systemV_shop_settings.h
	$(CC) main.c utils.c -o main 

receiver: receiver.c utils.c utils.h systemV_shop_settings.h
	$(CC) receiver.c utils.c -o receiver 

clean:
	rm -f main
