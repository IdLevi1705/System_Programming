CC=gcc 
CFLAGS=-Wall

all: mymalloc
mymalloc: mymalloc.o memgrind.o
mymalloc.o: mymalloc.c mymalloc.h
memgrind: memgrind.o mymalloc.h


clean:
	rm -f *.o *.txt mymalloc
run: mymalloc
	./mymalloc