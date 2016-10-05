CC=gcc
CFLAGS=-Wall

files: send receive

send: writenoncanonical.c
	$(CC) writenoncanonical.c -o send $(CFLAGS)

receive: noncanonical.c
	$(CC) noncanonical.c -o receive $(CFLAGS)
