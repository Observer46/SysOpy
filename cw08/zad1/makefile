# Makefile
CC = gcc -Wall

test:
	make times

times:
	make clean	
	make main
	touch Times.txt
	mkdir output

	make sign
	make block
	make interleaved

sign: main
	./main 1 sign baboon.ascii.pgm output/sign1.txt
	./main 2 sign baboon.ascii.pgm output/sign2.txt
	./main 4 sign baboon.ascii.pgm output/sign4.txt
	./main 8 sign baboon.ascii.pgm output/sign8.txt	

block: main
	./main 1 block baboon.ascii.pgm output/block1.txt
	./main 2 block baboon.ascii.pgm output/block2.txt
	./main 4 block baboon.ascii.pgm output/block4.txt
	./main 8 block baboon.ascii.pgm output/block8.txt

interleaved: main
	./main 1 interleaved baboon.ascii.pgm output/interleaved1.txt
	./main 2 interleaved baboon.ascii.pgm output/interleaved2.txt
	./main 4 interleaved baboon.ascii.pgm output/interleaved4.txt
	./main 8 interleaved baboon.ascii.pgm output/interleaved8.txt

main: main.c 
	$(CC) main.c -o main -lpthread -lm
	
clean:
	rm -f main
	rm -fdR output

	

