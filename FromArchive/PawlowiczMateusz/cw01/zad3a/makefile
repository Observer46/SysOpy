# Makefile
CC = gcc -Wall

testAll: clean
	printf "Statycznie: \n" >> results3a.txt
	make testStatic

	printf "Wspoldzielone: \n" >> results3a.txt
	make testShared

	printf "Dynamicznie: \n" >> results3a.txt
	make testDynamic

static:
	$(CC) -c myLib.c
	ar crs libmyLib.a myLib.o
	$(CC) main.c -o main -L. -lmyLib
	
shared:
	$(CC) -fPIC -shared myLib.c -o libmyLib.so
	$(CC) main.c -L. -Wl,-rpath=. -lmyLib -o main

dynamic: shared
	$(CC) mainDynamic.c -ldl -o mainDynamic

testStatic: static
	./main $(shell cat ../zad2/args.txt)

testShared: shared
	./main $(shell cat ../zad2/args.txt)

testDynamic: dynamic
	./mainDynamic $(shell cat ../zad2/args.txt)

clean:
	rm -f main
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f results3a.txt



