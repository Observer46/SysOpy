CC = gcc -Wall

test:
	make all
	./main

all:
	make receiver
	make wrapper
	make sender
	make main

main: main.c utils.c utils.h posix_shop_settings.h receiver wrapper sender
	$(CC) main.c utils.c -o main -lpthread -lrt

receiver: receiver.c utils.c utils.h posix_shop_settings.h
	$(CC) receiver.c utils.c -o receiver -lpthread -lrt

wrapper: wrapper.c utils.c utils.h posix_shop_settings.h
	$(CC) wrapper.c utils.c -o wrapper -lpthread -lrt

sender: sender.c utils.c utils.h posix_shop_settings.h
	$(CC) sender.c utils.c -o sender -lpthread -lrt

clean:
	rm -f main receiver wrapper sender
