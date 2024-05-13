CC=gcc
CFLAGS=-Wall -g -pthread

all: build

build: main.o
	$(CC) $(CFLAGS) main.o -o ex1

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o ex1

