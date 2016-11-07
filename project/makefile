CC=gcc
CFLAGS=-Wall -I.

nserial: interface.c data_link_layer.o application_layer.o
	$(CC) interface.c data_link_layer.o application_layer.o -o nserial $(CFLAGS)

clean:
	rm -f nserial *.o
