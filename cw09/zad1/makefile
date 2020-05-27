# Makefile
CC = gcc -Wall

test:
	make main
	./main 3 13

main: main.c 
	$(CC) main.c -o main -lpthread
	
clean:
	rm -f main	