CC = gcc -Wall

test:
	make all
	make automaticLaunch

	./automaticLaunch 2000 kill
	./automaticLaunch 10000 kill

	./automaticLaunch 2000 sigqueue
	./automaticLaunch 10000 sigqueue

	./automaticLaunch 2000 sigrt
	./automaticLaunch 10000 sigrt

all:
	make catcher
	make sender

catcher: catcher.c
	$(CC) catcher.c -o catcher
	
sender: sender.c
	$(CC) sender.c -o sender

automaticLaunch: main.c
	$(CC) main.c -o automaticLaunch

clean:
	rm -f catcher sender automaticLaunch
	