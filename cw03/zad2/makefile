# Makefile
CC = gcc -Wall

.PHONY: clean finder


macierz: main.c
	$(CC) main.c -o macierz

matrixHelper: matrixHelper.c
	$(CC) matrixHelper.c -o matrixHelper
	
clean:
	rm -f *.o
	rm -f matrix*.txt
	rm -f matrixHelper

	

