# Makefile
CC = gcc -Wall

.PHONY: clean finder

test: 
	make clean
	make finder

	mkdir test1
	mkdir test1/test2
	mkdir test1/test2/test3
	mkdir test1/test2/test4
	mkdir test5

	touch test1/test2/test3/file1
	touch test1/file2
	touch file3
	touch test1/test2/file4
	touch test5/file5

	ln -s test1/test2 slink1
	ln -s test1/test2/test3 slink2
	ln -s test5 slink3

	find -atime -1 -maxdepth 3 2> /dev/null | wc -l
	./finder -atime -1 -maxdepth 3 2> /dev/null | wc -l
	./finder -atime -1 -maxdepth 3 -nftw 2> /dev/null | wc -l
	echo -e '\n'
	find -atime 0 -maxdepth 1 2> /dev/null | wc -l
	./finder -atime 0 -maxdepth 1 2> /dev/null | wc -l
	./finder -atime 0 -maxdepth 1 -nftw 2> /dev/null | wc -l
	echo -e '\n'
	find /etc -atime 1 -mtime +1 -maxdepth 2 2> /dev/null | wc -l
	./finder /etc -atime 1 -mtime +1 -maxdepth 2 2> /dev/null | wc -l
	./finder /etc -atime 1 -mtime +1 -maxdepth 2 -nftw 2> /dev/null | wc -l

finder: main.c
	$(CC) main.c -o finder 
		
clean:
	rm -f *.o
	rm -fR test*
	rm -f slink*
	rm -f file*
	rm -f finder

	

