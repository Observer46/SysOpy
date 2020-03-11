# Makefile
CC = gcc -Wall

static: clean
	$(CC) -c myLib.c
	ar crs libmyLib.a myLib.o
	$(CC) cw01.c -o cw01 -L. -lmyLib
	
shard:	clean
	
clean:
	rm -f cw01
	rm -f *.o
	rm -f *.a
	

test:
	rm -f testo
	$(CC) testo.c -o testo
