CC=gcc
CFLAGS=-Wall -I.

files: send receive erase

send: writenoncanonical.c common.o
	$(CC) writenoncanonical.c common.o -o send $(CFLAGS)

receive: noncanonical.c common.o
	$(CC) noncanonical.c common.o -o receive $(CFLAGS)

clean:
	rm -f send receive common.o

erase:
	rm -f common.o
