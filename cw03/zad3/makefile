# Makefile
CC = gcc -Wall

test:
	make clean
	make all
	./matrixHelper generate 10 10 20
	./macierz listAll.txt 6 2 shared 1 3

	for idx in 0 1 2 3 4 5 6 7 8 9; do \
			./matrixHelper check matrix$${idx}A.txt matrix$${idx}B.txt matrix$${idx}C.txt;\
		done ;\

	./macierz listAll.txt 7 3 separated 2 2

	for idx in 0 1 2 3 4 5 6 7 8 9; do \
			./matrixHelper check matrix$${idx}A.txt matrix$${idx}B.txt matrix$${idx}C.txt;\
		done ;\

all:
	make matrixHelper
	make macierz

macierz: main.c matrixMethods.c
	$(CC) matrixMethods.c -o matrixMethods.o -c
	$(CC) main.c -o macierz.o -c
	$(CC) -o macierz macierz.o matrixMethods.o

matrixHelper: matrixHelper.c matrixMethods.c
	$(CC) matrixMethods.c -o matrixMethods.o -c
	$(CC) matrixHelper.c -o matrixHelper.o -c
	$(CC) -o matrixHelper matrixHelper.o matrixMethods.o
	
clean:
	rm -f *.o
	rm -f *matrix*.txt
	rm -f matrixHelper
	rm -f macierz
	rm -f list*.txt

	

