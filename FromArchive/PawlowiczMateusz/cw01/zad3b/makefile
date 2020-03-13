# Makefile
CC = gcc -Wall
O = O0

testAll: fullClean
	printf "Statycznie O0: \n" >> results3b.txt
	make testStatic O=O0

	printf "Wspoldzielone O0: \n" >> results3b.txt
	make testShared O=O0

	printf "Dynamicznie O0: \n" >> results3b.txt
	make testDynamic O=O0

	printf "\n\n\n\n" >> results3b.txt

	printf "Statycznie O1: \n" >> results3b.txt
	make testStatic O=O1

	printf "Wspoldzielone O1: \n" >> results3b.txt
	make testShared O=O1

	printf "Dynamicznie O1: \n" >> results3b.txt
	make testDynamic O=O1


	printf "\n\n\n\n" >> results3b.txt


	printf "Statycznie O2: \n" >> results3b.txt
	make testStatic O=O2

	printf "Wspoldzielone O2: \n" >> results3b.txt
	make testShared O=O2

	printf "Dynamicznie O2: \n" >> results3b.txt
	make testDynamic O=O2

static: clean
	$(CC) -c myLib.c
	ar crs libmyLib.a myLib.o
	$(CC) main.c -$O -o main -L. -lmyLib
	
shared: clean
	$(CC) -fPIC -shared myLib.c -o libmyLib.so
	$(CC) main.c -$O -L. -Wl,-rpath=. -lmyLib -o main

dynamic: shared
	$(CC) mainDynamic.c -$O -ldl -o mainDynamic

testStatic: static
	./main $(shell cat ../zad2/args.txt)

testShared: shared
	./main $(shell cat ../zad2/args.txt)

testDynamic: dynamic
	./mainDynamic $(shell cat ../zad2/args.txt)


clean:
	rm -f main
	rm -f mainDynamic
	rm -f *.o
	rm -f *.a
	rm -f *.so

fullClean:
	rm -f main
	rm -f mainDynamic
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f results3b.txt



