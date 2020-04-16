CC = gcc -Wall

test:
	make main
	./main sample_text.txt

main: main.c
	$(CC) main.c -o main

clean:
	rm -f main
