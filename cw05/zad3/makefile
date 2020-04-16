CC = gcc -Wall

test:
	make all
	./main pipe 5 10 3 output.txt

all:
	make producer
	make consumer
	make main

main: main.c
	$(CC) main.c -o main

producer: producer.c
	$(CC) producer.c -o producer

consumer: consumer.c
	$(CC) consumer.c -o consumer

testo: testo.c
	$(CC) testo.c -o testo

clean:
	rm -f main producer consumer
	rm -f input_producer*
	rm -f pipe
	rm -f output.txt

