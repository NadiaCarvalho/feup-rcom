CC=gcc
CFLAGS=-Wall -I.

files: send receive interface erase

send: writenoncanonical.c data_link_layer.o application_layer.o
	$(CC) writenoncanonical.c data_link_layer.o application_layer.o -o send $(CFLAGS)

receive: noncanonical.c data_link_layer.o application_layer.o
	$(CC) noncanonical.c data_link_layer.o application_layer.o -o receive $(CFLAGS)

interface: interface.c data_link_layer.o application_layer.o
	$(CC) interface.c data_link_layer.o application_layer.o -o interface $(CFLAGS)

clean:
	rm -f send receive interface data_link_layer.o

erase:
	rm -f common.o data_link_layer.o
