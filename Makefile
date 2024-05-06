CC=gcc
CFLAGS=-Wall -g

all: teste

teste: teste.o
	$(CC) $(CFLAGS) teste.o -o teste

teste.o: teste.c
	$(CC) $(CFLAGS) -c teste.c

clean:
	rm -f *.o teste
