CC=gcc
CFLAGS=-Wall -I.

files: nserial

send: writenoncanonical.c data_link_layer.o application_layer.o
	$(CC) writenoncanonical.c data_link_layer.o application_layer.o -o send $(CFLAGS)

receive: noncanonical.c data_link_layer.o application_layer.o
	$(CC) noncanonical.c data_link_layer.o application_layer.o -o receive $(CFLAGS)

nserial: interface.c data_link_layer.o application_layer.o
	$(CC) interface.c data_link_layer.o application_layer.o -o nserial $(CFLAGS)

clean:
	rm -f send receive nserial data_link_layer.o

erase:
	rm -f common.o data_link_layer.o
