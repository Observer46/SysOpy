CC = gcc -Wall

test:
	make all

	./main ignore fork
	./main handler fork
	./main mask fork
	./main pending fork

	./main ignore exec
	./main mask exec
	./main pending exec

all:
	make main
	make execTest

main: main.c
	$(CC) main.c -o main
	
execTest: execTest.c
	$(CC) execTest.c -o execTest

clean:
	rm -f main
	rm -f execTest