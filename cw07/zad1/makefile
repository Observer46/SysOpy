CC = gcc -Wall

test:
	make all
	./main

all:
	make receiver
	make wrapper
	make sender
	make main

main: main.c utils.c utils.h systemV_shop_settings.h receiver wrapper sender
	$(CC) main.c utils.c -o main 

receiver: receiver.c utils.c utils.h systemV_shop_settings.h
	$(CC) receiver.c utils.c -o receiver

wrapper: wrapper.c utils.c utils.h systemV_shop_settings.h
	$(CC) wrapper.c utils.c -o wrapper

sender: sender.c utils.c utils.h systemV_shop_settings.h
	$(CC) sender.c utils.c -o sender

clean:
	rm -f main receiver wrapper sender
