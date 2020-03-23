# Makefile
CC = gcc -Wall

.PHONY: clean program test

test: program
	make clean
	make program

	for recordSize in 1 4 512 1024 4096 8192; do \
		for recordCount in 10000 20000; do \
			./program generate dane_$${recordCount}_$$recordSize $$recordCount $$recordSize ;\
			printf "Kopiowanie sys: $$recordCount rekordow rozmiaru $$recordSize \n" >> wyniki.txt ;\
			./program copy dane_$${recordCount}_$$recordSize dane_$${recordCount}_$${recordSize}_sys $$recordCount $$recordSize sys ;\
			printf "Kopiowanie lib: $$recordCount rekordow rozmiaru $$recordSize \n" >> wyniki.txt ;\
			./program copy dane_$${recordCount}_$$recordSize dane_$${recordCount}_$${recordSize}_lib $$recordCount $$recordSize lib ;\
		done ;\
	done
	printf "\n\n\n" >> wyniki.txt
	for recordSize in 1 4 512 1024 4096 8192; do \
		for recordCount in 10000 20000; do \
			printf "Sortowanie sys: $$recordCount rekordow rozmiaru $$recordSize \n" >> wyniki.txt ;\
			./program sort dane_$${recordCount}_$${recordSize}_sys $$recordCount $$recordSize sys ;\
			printf "Sortowanie lib: $$recordCount rekordow rozmiaru $$recordSize \n" >> wyniki.txt ;\
			./program sort dane_$${recordCount}_$${recordSize}_lib $$recordCount $$recordSize lib ;\
		done ;\
	done


program: main.c
	$(CC) generate.c -o generate.o -c
	$(CC) sort.c -o sort.o -c
	$(CC) copy.c -o copy.o -c
	$(CC) main.c -o main.o -c
	$(CC) -o program main.o generate.o sort.o copy.o
		
clean:
	rm -f *.o
	rm -f program
	rm -f dane*

	

