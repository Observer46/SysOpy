# Makefile
CC = gcc -Wall

static: clean
	$(CC) -c myLib.c
	ar crs libmyLib.a myLib.o
	$(CC) main.c -o main -L. -lmyLib
	
shared:	clean
	$(CC) -fPIC -shared myLib.c -o libmyLib.so
	$(CC) main.c -L. -Wl,-rpath=. -lmyLib -o main
	
clean:
	rm -f main
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f raport2.txt



