CC = gcc -Wall

test:
	make main
	./main command_test.txt

main: main.c
	$(CC) main.c -o main

clean:
	rm -f main
