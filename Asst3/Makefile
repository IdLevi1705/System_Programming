server: DUMBserver.c
	gcc -m32 -o server DUMBserver.c -I. -lm -ldl -pthread

client: DUMBclient.c
	gcc -m32 -o client DUMBclient.c -I. -lm -ldl -pthread 


.PHONY: clean

clean:
	rm -f server client *.o


all: server client
