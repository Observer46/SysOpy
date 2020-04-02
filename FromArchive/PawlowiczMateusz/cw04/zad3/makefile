CC = gcc -Wall

test:
	make main

	
	./main child
	./main child_int
	./main div


appendToReport: # Tak jest szybciej niz w kodzie 
	make main

	./main div >> raport3.txt
	./main child >> raport3.txt
	./main child_int >> raport3.txt


main: main.c
	$(CC) main.c -o main

clean:
	rm -f main